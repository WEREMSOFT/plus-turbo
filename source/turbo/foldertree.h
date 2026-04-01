#ifndef TURBO_FOLDERTREE_H
#define TURBO_FOLDERTREE_H

#define Uses_TWindow
#define Uses_TOutline
#include <tvision/tv.h>

#include <variant>
#include <string_view>

struct EditorWindow;

struct FolderTreeView : public TOutline {

    struct FolderNode : public TNode {

        TNode **ptr;
        FolderNode *parent;
        std::variant<std::string, EditorWindow *> data;
		std::string fullPath;
        FolderNode(FolderNode *parent, std::string_view path) noexcept;
        FolderNode(FolderNode *parent, EditorWindow *w) noexcept;
        bool hasEditor() const noexcept;
        EditorWindow* getEditor() noexcept;
        void remove() noexcept;
        void dispose() noexcept;

    };

    bool focusing {false};

    using TOutline::TOutline;

    void focused(int i) noexcept override;

    void addEditor(EditorWindow *w) noexcept;
    void addFileNode(std::string filePath) noexcept;
    void focusEditor(EditorWindow *w) noexcept;
    void removeEditor(EditorWindow *w) noexcept;
    void focusNext() noexcept;
    void focusPrev() noexcept;
	void reset() noexcept;
	void loadFromFolder(const std::string &path) noexcept;
	void loadRecursive(const std::string &path) noexcept;
	void deleteSubtree(TNode *node) noexcept;
    FolderNode *getDirNode(std::string_view dirPath) noexcept;
    FolderNode *findByEditor(const EditorWindow *w, int *pos=nullptr) noexcept;
    FolderNode *findByPath(std::string_view path) noexcept;
    template <class Func>
    FolderNode *firstThat(Func &&func) noexcept;
};

template <class Func>
inline FolderTreeView::FolderNode *FolderTreeView::firstThat(Func &&func) noexcept
{
    auto applyCallback =
    [] ( TOutlineViewer *, TNode *node, int, int position,
         long, ushort, void *arg )
    {
        return (*(Func *) arg)((FolderNode *) node, position);
    };

    return (FolderNode *) TOutlineViewer::firstThat(applyCallback, &func);
}

struct FolderTreeWindow : public TWindow {

    FolderTreeView *tree;
    FolderTreeWindow **ptr;

    FolderTreeWindow(const TRect &bounds, FolderTreeWindow **ptr) noexcept;
    ~FolderTreeWindow();

    void close() override;

};

template <class Func>
inline TNode* findInFolderList(TNode **list, Func &&test)
{
    auto *node = *list;
    while (node) {
        auto *next = node->next;
        if (test((FolderTreeView::FolderNode *) node))
            return node;
        node = next;
    }
    return nullptr;
}

#endif
