#include <stdio.h>
int value[10000];

int left_child[10000];

int right_child[10000];

int now;

int ret;
int n;
int readn;
int i;
int root;


int search(int root, int x);
int find_minimum(int root);
int new_node(int x);
int insert(int root, int x);
int delete_node(int root, int x);
void inorder(int root);

int search(int root, int x) {
    int _retval;
    if (((root == (-1)) || (value[root] == x))) {
        _retval = root;
    } else {
        if ((x > value[root])) {
            _retval = search(right_child[root], x);
        } else {
            _retval = search(left_child[root], x);
        }
    }

    return _retval;
}

int find_minimum(int root) {
    int _retval;
    if ((root == (-1))) {
        _retval = (-1);
    } else {
        if ((left_child[root] != (-1))) {
            _retval = find_minimum(left_child[root]);
        } else {
            _retval = root;
        }
    }

    return _retval;
}

int new_node(int x) {
    int _retval;
    value[now] = x;
    left_child[now] = (-1);
    right_child[now] = (-1);
    _retval = now;
    now = (now + 1);

    return _retval;
}

int insert(int root, int x) {
    int _retval;
    if ((root == (-1))) {
        _retval = new_node(x);
    } else {
        if ((x > value[root])) {
            right_child[root] = insert(right_child[root], x);
        } else {
            left_child[root] = insert(left_child[root], x);
        }
    }
    _retval = root;

    return _retval;
}

int delete_node(int root, int x) {
    int _retval;
    int tmp;

    if ((x > value[root])) {
        right_child[root] = delete_node(right_child[root], x);
    } else {
        if ((x < value[root])) {
            left_child[root] = delete_node(left_child[root], x);
        } else {
            if (((left_child[root] == (-1)) && (right_child[root] == (-1)))) {
                _retval = (-1);
            } else {
                if (((left_child[root] == (-1)) || (right_child[root] == (-1)))) {
                    if ((left_child[root] == (-1))) {
                        _retval = right_child[root];
                    } else {
                        _retval = left_child[root];
                    }
                } else {
                    tmp = find_minimum(right_child[root]);
                    value[root] = value[tmp];
                    right_child[root] = delete_node(right_child[root], value[tmp]);
                }
            }
        }
    }
    _retval = root;

    return _retval;
}

void inorder(int root) {
    if ((root != (-1))) {
        inorder(left_child[root]);
        printf("%d%c", value[root], ' ');
        inorder(right_child[root]);
    }

}


int main(void) {
    ret = 0;
    now = 0;
    scanf("%d", &n);
    if ((n == 0)) {
        ret = 0;
    }
    scanf("%d", &readn);
    root = new_node(readn);
    for (i = 1; i <= (n - 1); i++) {
        scanf("%d", &readn);
        insert(root, readn);
    }
    inorder(root);
    scanf("%d", &n);
    for (i = 1; i <= n; i++) {
        scanf("%d", &readn);
        root = delete_node(root, readn);
    }
    inorder(root);

    return 0;
}
