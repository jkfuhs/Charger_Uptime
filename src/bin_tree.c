#include "bin_tree.h"

int max(int a, int b)
{
    return (a > b ? a : b);
}

int height(bin_tree N)
{
    return (N ? N->height : 0);
}

// Get balance from tree node N
int getBalance(bin_tree N)
{
    if (!N)
    {
        return 0;
    }
    return height(N->left) - height(N->right);
}


// utility function to create a new node
bin_tree new_node(uint64_t d1, uint64_t d2, bin_tree runtime, bin_tree uptime)
{
    bin_tree new_node = malloc(sizeof(struct bin_tree_node_st));
    if (!new_node)
    {
        perror("could not allocate new node");
        return NULL;
    }

    new_node->d1 = d1;
    new_node->d2 = d2;
    new_node->left = NULL;
    new_node->right = NULL;
    new_node->height = 1;
    new_node->runtime = runtime;
    new_node->uptime = uptime;
    return new_node;

}

// utility function to rebalance tree by rotating a subtree
bin_tree rightRotate(bin_tree y)
{
    bin_tree x = y->left;
    bin_tree T2 = x->right;

    //rotate
    x->right = y;
    y->left = T2;

    // Update heights
    y->height = max(height(y->left), height(y->right)) + 1;
    x->height = max(height(x->left), height(x->right)) + 1;

    // return root
    return x;
}

bin_tree leftRotate(bin_tree x)
{
    bin_tree y = x->right;
    bin_tree T2 = y->left;

    // rotate
    y->left = x;
    x->right = T2;

    //Update heights
    x->height = max(height(x->left), height(x->right)) + 1;
    y->height = max(height(y->left), height(y->right)) + 1;

    //return root
    return y;
}

bin_tree add_node(bin_tree head, uint64_t d1, uint64_t d2, bin_tree runtime, bin_tree uptime)
{
    if (!head)
    {
        return new_node(d1, d2, NULL, NULL);
    }
    
    if (d1 < head->d1)
    {
        head->left = add_node(head->left, d1, d2, runtime, uptime);
    }else
    {
        head->right = add_node(head->right, d1, d2, runtime, uptime);
    }

    // Update height
    head->height = max(height(head->left), height(head->right)) + 1;

    int balance = getBalance(head);

    // 4 cases:
    //Left Left case
    if (balance > 1 && d1 < head->left->d1)
    {
        return rightRotate(head);
    }

    // Right Right case
    if (balance < -1 && d1 > head->right->d1)
    {
        return leftRotate(head);
    }

    // Left Right case
    if (balance > 1 && d1 > head->left->d1)
    {
        head->left = leftRotate(head->left);
        return rightRotate(head);
    }

    // Right Left case
    if (balance < -1 && d1 < head->right->d1)
    {
        head->right = rightRotate(head->right);
        return leftRotate(head);
    }


    // otherwise, return the unchanged tree
    return head;

}

bin_tree get_node(bin_tree head, uint64_t key)
{
    if (!head)
    {
        return NULL;
    }
    if (head->d1 == key)
    {
        return head;
    }
    if (key < head->d1)
    {
        return get_node(head->left, key);
    }
    if (key > head->d1)
    {
        return get_node(head->right, key);
    }

    // Should not get here, but a failsafe
    return NULL;
}

void pre_print(bin_tree N)
{
    if (!N)
    {
        return;
    }
    pre_print(N->left);
    printf("%ld %ld\n", N->d1, N->d2);
    pre_print(N->right);
}

void free_tree(bin_tree tree)
{
    if (!tree)
    {
        return;
    }
    free_tree(tree->left);
    free_tree(tree->right);
    free(tree);
    return;
}