#include "foldertree.h"
#include "editwindow.h"
#include "app.h"
#include <utility>
#include <cassert>
#include <turbo/tpath.h>
#include <filesystem>

#define Uses_TProgram
#include <tvision/tv.h>

using FolderNode = FolderTreeView::FolderNode;

FolderNode::FolderNode(FolderNode *parent, std::string_view p) noexcept :
    TNode(TPath::basename(p)),
    ptr(nullptr),
    parent(parent),
    data(std::string {p}),
	fullPath(std::string {p}) 
{
}

FolderNode::FolderNode(FolderNode *parent, EditorWindow *w) noexcept :
    TNode(w->getTitle()),
    ptr(nullptr),
    parent(parent),
    data(w)
{
}

bool FolderNode::hasEditor() const noexcept
{
    return std::holds_alternative<EditorWindow *>(data);
}

EditorWindow* FolderNode::getEditor() noexcept
{
    if (auto **pw = std::get_if<EditorWindow *>(&data))
        return *pw;
    return nullptr;
}

void FolderNode::remove() noexcept
{
    if (next)
        ((FolderNode *) next)->ptr = ptr;
    if (ptr)
        *ptr = next;
    next = nullptr;
    ptr = nullptr;
    if (parent && !parent->childList)
        parent->dispose();
    parent = nullptr;
}

void FolderNode::dispose() noexcept
{
    assert(!childList);
    remove();
    delete this;
}

// Directories shall appear before files.
enum FolderNodeType { fntDir, fntEditor };
using FolderNodeKey = std::pair<FolderNodeType, std::string_view>;

static FolderNodeKey getKey(FolderNode *node) noexcept
{
    if (node->hasEditor())
        return {fntEditor, node->text};
    return {fntDir, node->text};
}

static void putNode(TNode **indirect, FolderNode *node) noexcept
// Pre: node->parent is properly set.
{
    auto key = getKey(node);
    TNode *other;
    while ((other = *indirect))
    {
        if (key < getKey((FolderNode *) other))
        {
            node->next = other;
            ((FolderNode *) other)->ptr = &node->next;
            break;
        }
        indirect = &other->next;
    }
    *indirect = node;
    node->ptr = indirect;
}

static void setParent(FolderNode *node, FolderNode *parent) noexcept
{
    if (!parent)
        node->dispose();
    else if (parent != node->parent)
    {
        node->remove(); // May free the parent, hence the 'parent != node->parent' check.
        node->parent = parent;
        putNode(&parent->childList, node);
    }
}

void FolderTreeView::focused(int i) noexcept
{
    // Avoid reentrancy on focus:
    if (!focusing)
    {
        focusing = true;
        TOutline::focused(i);
        if (auto *node = (FolderNode *) getNode(i))
		{
            if (auto *w = node->getEditor())
			{
                w->focus();
			} else {
				namespace fs = std::filesystem;
				if(node->fullPath != NULL && fs::is_regular_file(node->fullPath))
				{
					message(TProgram::application, evCommand, 	cmFolderTreeClick, (void *)node);
				}
			}
		}
        update();
        focusing = false;
    }
}

void FolderTreeView::addEditor(EditorWindow *w) noexcept
{
    FolderNode *parent;
    TNode **list;
    if (w->filePath().empty()) {
        parent = nullptr;
        list = &root;
    } else {
        parent = getDirNode(TPath::dirname(w->filePath()));
        list = &parent->childList;
    }
    putNode(list, new FolderNode(parent, w));
    update();
    drawView();
}

void FolderTreeView::addFileNode(std::string filePath) noexcept
{
    FolderNode *parent;
    TNode **list;
    if (filePath.empty()) {
        parent = nullptr;
        list = &root;
    } else {
        parent = getDirNode(TPath::dirname(filePath));
        list = &parent->childList;
    }
    putNode(list, new FolderNode(parent, filePath));
    update();
    drawView();
}

void FolderTreeView::focusEditor(EditorWindow *w) noexcept
{
    // Avoid the reentrant case (see focused()).
    if (focusing)
        return;
    int i;
    if (findByEditor(w, &i))
        focused(i);
    drawView();
}

void FolderTreeView::removeEditor(EditorWindow *w) noexcept
{
    FolderNode *f = findByEditor(w);
    if (f) {
		f->data = nullptr;
        update();
        drawView();
    }
}

void FolderTreeView::focusNext() noexcept
{
    firstThat([this] (auto *node, auto pos) {
        if (((FolderNode *) node)->hasEditor() && pos > foc) {
            focused(pos);
            drawView();
            return true;
        }
        return false;
    });
}

void FolderTreeView::focusPrev() noexcept
{
    int prevPos = -1;
    firstThat([this, &prevPos] (auto *node, auto pos) {
        if (((FolderNode *) node)->hasEditor()) {
            if (pos < foc)
                prevPos = pos;
            else if (prevPos >= 0) {
                focused(prevPos);
                drawView();
                return true;
            }
        }
        return false;
    });
}

FolderNode* FolderTreeView::getDirNode(std::string_view dirPath) noexcept
{
    // The list where the dir will be inserted.
    TNode **list {nullptr};
    FolderNode *parent {nullptr};
    {
        auto parentPath = TPath::dirname(dirPath);
        if ((parent = findByPath(parentPath)))
            list = &parent->childList;
    }
    if (!list)
        list = &root;
    // The directory we are searching for.
    auto *dir = (FolderNode *) findInFolderList(list, [dirPath] (FolderNode *node) {
        auto *ppath = std::get_if<std::string>(&node->data);
        return ppath && *ppath == dirPath;
    });
    if (!dir) {
        dir = new FolderNode(parent, dirPath);
        // Place already existing subdirectories under this dir.
        findInFolderList(&root, [dir, dirPath] (FolderNode *node) {
            auto *ppath = std::get_if<std::string>(&node->data);
            if (ppath && TPath::dirname(*ppath) == TStringView(dirPath))
                setParent(node, dir);
            return false;
        });
        putNode(list, dir);
    }
    return dir;
}

FolderNode* FolderTreeView::findByEditor(const EditorWindow *w, int *pos) noexcept
{
    return firstThat(
    [w, pos] (FolderNode *node, int position)
    {
        auto *w_ = node->getEditor();
        if (w_ && w_ == w) {
            if (pos)
                *pos = position;
            return true;
        }
        return false;
    });
}


FolderNode* FolderTreeView::findByPath(std::string_view path) noexcept
{
    return firstThat(
    [path] (FolderNode *node, int)
    {
        auto *ppath = std::get_if<std::string>(&node->data);
        return ppath && *ppath == path;
    });
}

FolderTreeWindow::FolderTreeWindow(const TRect &bounds, FolderTreeWindow **ptr) noexcept :
    TWindowInit(&FolderTreeWindow::initFrame),
    TWindow(bounds, "Files In Folder", wnNoNumber),
    ptr(ptr)
{
    auto *hsb = standardScrollBar(sbHorizontal),
         *vsb = standardScrollBar(sbVertical);
    tree = new FolderTreeView(getExtent().grow(-1, -1), hsb, vsb, nullptr);
    tree->growMode = gfGrowHiX | gfGrowHiY;
    insert(tree);
}

FolderTreeWindow::~FolderTreeWindow()
{
    if (ptr)
        *ptr = nullptr;
}

void FolderTreeWindow::close()
{
    message(TProgram::application, evCommand, cmToggleFolderTree, 0);
}
