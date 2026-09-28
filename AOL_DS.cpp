#include <stdio.h> // library yang digunakan
#include <stdlib.h> // library yang digunakan
#include <string.h> // library yang digunakan
#include <stdbool.h> // library yang digunakan

#define MIN_DEGREE 3 // ini merukanan nilai minimum (t) dari B-Tree
// rumus node punya maksimal (2t - 1)key
// rumus node punya minimal (t - 1)key

typedef struct StudentRecord { // ini merupakan stuktur data yang menyimpan data mahasiswa
    char studentID[11]; 
    char name[52];
    char major[52];
    float gpa;
} StudentRecord;

typedef struct BTreeNode { // ini untk B-Tree node nya
    int n; // jumblah key dalam node
    char (*keys)[10]; // ini array key
    StudentRecord *records; // ini array data mahasiswa
    struct BTreeNode **children; // array pointer ke children
    bool isLeaf; // check apakah ini leaf
    int t; // degree B-Tree
} BTreeNode;

typedef struct BTree {
    BTreeNode *root;
    int t;
    int id_length;
} BTree;

BTree *studentTree;

// ini untuk membuat node baru
BTreeNode* createNode(int t_val, bool leaf_val) {
    BTreeNode* newNode =  (BTreeNode*) malloc(sizeof(BTreeNode));
    newNode->t = t_val;
    newNode->isLeaf = leaf_val;
    newNode->n = 0;
    newNode->keys = (char (*)[10])malloc(sizeof(char[10]) * (2 * t_val - 1));
    newNode->records = (StudentRecord*) malloc(sizeof(StudentRecord) * (2 * t_val - 1));
    newNode->children = (BTreeNode**) malloc(sizeof(BTreeNode*) * (2 * t_val));
    return newNode;
}

// ini untuk inalisasi B-Tree
BTree* createBTree(int t_val, int id_len) {
    BTree* tree = (BTree*)malloc(sizeof(BTree));
    tree->root = NULL;
    tree->t = t_val;
    tree->id_length = id_len;
    return tree;
}

// fungsi ini untuk mencari indeks untuk key
int findKeyIndex(BTreeNode *node, const char *key) {
    int idx = 0;
    while (idx < node->n && strcmp(node->keys[idx], key) < 0) idx++;
    return idx;
}

// untuk mendapatkan predecessor 
char* getPred(BTreeNode *node, int idx) {
    BTreeNode *cur = node->children[idx];
    while (!cur->isLeaf) cur = cur->children[cur->n];
    return cur->keys[cur->n - 1];
}
// untuk mendapatkan successor
char* getSucc(BTreeNode *node, int idx) {
    BTreeNode *cur = node->children[idx + 1];
    while (!cur->isLeaf) cur = cur->children[0];
    return cur->keys[0];
}

// fungsi untuk meminjam 1 data dari siblings kiri ke child yang kekurangan data
// saat node dalam B-Tree kekurangan isi 
void borrowFromPrev(BTreeNode *node, int idx) {
    BTreeNode *child = node->children[idx];
    BTreeNode *sib = node->children[idx - 1];

    for (int i = child->n - 1; i >= 0; i--) {
        strcpy(child->keys[i + 1], child->keys[i]);
        child->records[i + 1] = child->records[i];
    }
    if (!child->isLeaf) {
        for (int i = child->n; i >= 0; i--)
            child->children[i + 1] = child->children[i];
    }
    strcpy(child->keys[0], node->keys[idx - 1]);
    child->records[0] = node->records[idx - 1];
    if (!child->isLeaf)
        child->children[0] = sib->children[sib->n];
    strcpy(node->keys[idx - 1], sib->keys[sib->n - 1]);
    node->records[idx - 1] = sib->records[sib->n - 1];
    child->n++;
    sib->n--;
}
// fungsi untuk meminjam 1 data dari siblings kanan ke child yang kekurangan data
// saat node dalam B-Tree kekurangan isi setelah penghapusan data
void borrowFromNext(BTreeNode *node, int idx) {
    BTreeNode *child = node->children[idx];
    BTreeNode *sib = node->children[idx + 1];

    strcpy(child->keys[child->n], node->keys[idx]);
    child->records[child->n] = node->records[idx];
    if (!child->isLeaf)
        child->children[child->n + 1] = sib->children[0];

    strcpy(node->keys[idx], sib->keys[0]);
    node->records[idx] = sib->records[0];

    for (int i = 1; i < sib->n; i++) {
        strcpy(sib->keys[i - 1], sib->keys[i]);
        sib->records[i - 1] = sib->records[i];
    }
    if (!sib->isLeaf) {
        for (int i = 1; i <= sib->n; i++)
            sib->children[i - 1] = sib->children[i];
    }
    child->n++;
    sib->n--;
}

// digunakan untuk gabungin dua child yang isinya dikit jadi satu anak yang lebih penuh
// saat penghapusan data di B-Tree
void mergeChildren(BTreeNode *node, int idx) {
    BTreeNode *child = node->children[idx];
    BTreeNode *sib = node->children[idx + 1];

    strcpy(child->keys[child->n], node->keys[idx]);
    child->records[child->n] = node->records[idx];
    child->n++;
    for (int i = 0; i < sib->n; i++) {
        strcpy(child->keys[child->n + i], sib->keys[i]);
        child->records[child->n + i] = sib->records[i];
    }
    if (!child->isLeaf) {
        for (int i = 0; i <= sib->n; i++)
            child->children[child->n + i] = sib->children[i];
    }
    child->n += sib->n;

    for (int i = idx + 1; i < node->n; i++) {
       	strcpy(node->keys[i - 1], node->keys[i]);
        node->records[i - 1] = node->records[i];
    }
    for (int i = idx + 2; i <= node->n; i++)
        node->children[i - 1] = node->children[i];
    node->n--;
    free(sib->keys);
    free(sib->records);
    free(sib->children);
    free(sib);
}

// memastikan child memiliki cukup key
void fillChild(BTreeNode *node, int idx) {
    if (idx && node->children[idx - 1]->n >= node->t)
        borrowFromPrev(node, idx);
    else if (idx != node->n && node->children[idx + 1]->n >= node->t)
        borrowFromNext(node, idx);
    else {
        if (idx != node->n) mergeChildren(node, idx);
        else mergeChildren(node, idx - 1);
    }
}

// buat nyari 1 data mahasiswa berdasaryakn key
StudentRecord* search_node(BTreeNode *node, const char *key) {
    int i = 0;
    while (i < node->n && strcmp(key, node->keys[i]) > 0) i++;
    if (i < node->n && strcmp(key, node->keys[i]) == 0)
        return &node->records[i];
    if (node->isLeaf) return NULL;
    return search_node(node->children[i], key);
}

// tambahan buat cek pohonnya ada atau engga
StudentRecord* search_student(BTree *tree, const char *key) {
    if (!tree->root) return NULL;
    return search_node(tree->root, key);
}

// menampilkan seluruh data yang ada di dalam B-Tree secara berurutan
void traverse(BTreeNode *node) {
    int i;
    for (i = 0; i < node->n; i++) {
        if (!node->isLeaf) traverse(node->children[i]);
        printf("%-10s | %-20s | %-20s | %.2f\n",
               node->records[i].studentID,
               node->records[i].name,
               node->records[i].major,
               node->records[i].gpa);
    }
    if (!node->isLeaf) traverse(node->children[i]);
}

// fungsi kalau node di B-Tree sudah penuh, dan kita mau masukin data baru
// kita bagi node menjadi dua child
void splitChild(BTreeNode *parent, int idx, BTreeNode *child) {
    BTreeNode *newChild = createNode(child->t, child->isLeaf);
    newChild->n = child->t - 1;

    for (int j = 0; j < child->t - 1; j++) {
        strcpy(newChild->keys[j], child->keys[j + child->t]);
        newChild->records[j] = child->records[j + child->t];
    }
    if (!child->isLeaf) {
        for (int j = 0; j < child->t; j++)
            newChild->children[j] = child->children[j + child->t];
    }
    child->n = child->t - 1;
    for (int j = parent->n; j >= idx + 1; j--)
        parent->children[j + 1] = parent->children[j];
    parent->children[idx + 1] = newChild;
    for (int j = parent->n - 1; j >= idx; j--) {
       	strcpy(parent->keys[j + 1], parent->keys[j]);
        parent->records[j + 1] = parent->records[j];
    }
   	strcpy(parent->keys[idx], child->keys[child->t - 1]);
    parent->records[idx] = child->records[child->t - 1];
    parent->n++;
}

// memasukkan data baru ke dalam node B-Tree yang belum penuh
void insertNonFull(BTreeNode *node, const char *key, StudentRecord rec) {
    int i = node->n - 1;
    if (node->isLeaf) {
        while (i >= 0 && strcmp(node->keys[i], key) > 0) {
            strcpy(node->keys[i + 1], node->keys[i]);
            node->records[i + 1] = node->records[i];
            i--;
        }
        strcpy(node->keys[i + 1], key);
        node->records[i + 1] = rec;
        node->n++;
    } else {
        while (i >= 0 && strcmp(node->keys[i], key) > 0) i--;
        i++;
        if (node->children[i]->n == 2 * node->t - 1) {
            splitChild(node, i, node->children[i]);
            if (strcmp(node->keys[i], key) < 0) i++;
        }
        insertNonFull(node->children[i], key, rec);
    }
}

// memasukkan data mahasiswa baru ke dalam B-Tree
void insert_student(BTree *tree, const char *key, const char *name,
                    const char *major, float gpa) {
    StudentRecord rec;
    strncpy(rec.studentID, key, 10);
    strncpy(rec.name, name, 50);
    strncpy(rec.major, major, 50);
    rec.gpa = gpa;

    if (!tree->root) {
        tree->root = createNode(tree->t, true);
        strcpy(tree->root->keys[0], key);
        tree->root->records[0] = rec;
        tree->root->n = 1;
    } else {
        if (tree->root->n == 2 * tree->t - 1) {
            BTreeNode *s = createNode(tree->t, false);
            s->children[0] = tree->root;
            splitChild(s, 0, tree->root);
            int idx = strcmp(s->keys[0], key) < 0 ? 1 : 0;
            insertNonFull(s->children[idx], key, rec);
            tree->root = s;
        } else {
            insertNonFull(tree->root, key, rec);
        }
    }
}

// menghapus sebuah data (key) dari node B-Tree
void deleteFromNode(BTreeNode *node, const char *key) {
    int idx = findKeyIndex(node, key);

    if (idx < node->n && strcmp(node->keys[idx], key) == 0) {
        if (node->isLeaf) {
            for (int i = idx + 1; i < node->n; i++) {
                strcpy(node->keys[i - 1], node->keys[i]);
                node->records[i - 1] = node->records[i];
            }
            node->n--;
        } else {
            if (node->children[idx]->n >= node->t) {
                char *predKey = getPred(node, idx);
                StudentRecord predRec = *search_node(node->children[idx], predKey);
                strcpy(node->keys[idx], predKey);
                node->records[idx] = predRec;
                deleteFromNode(node->children[idx], predKey);
            }
            else if (node->children[idx + 1]->n >= node->t) {
                char *succKey = getSucc(node, idx);
                StudentRecord succRec = *search_node(node->children[idx + 1], succKey);
                strcpy(node->keys[idx], succKey);
                node->records[idx] = succRec;
                deleteFromNode(node->children[idx + 1], succKey);
            } else {
                mergeChildren(node, idx);
                deleteFromNode(node->children[idx], key);
            }
        }
    }
    else {
        if (node->isLeaf) return;
        bool flag = (idx == node->n);
        if (node->children[idx]->n < node->t)
            fillChild(node, idx);

        if (flag && idx > node->n)
            deleteFromNode(node->children[idx - 1], key);
        else
            deleteFromNode(node->children[idx], key);
    }
}

// untuk menjaga agar struktur pohon tetap benar dan konsisten setelah penghapusan data
void deleteKey(BTree *tree, const char *key) {
    if (!tree->root) return;
    deleteFromNode(tree->root, key);

    if (tree->root->n == 0) {
        BTreeNode *tmp = tree->root;
        if (tmp->isLeaf) {
            free(tmp->keys);
            free(tmp->records);
            free(tmp->children);
            free(tmp);
            tree->root = NULL;
        } else {
            tree->root = tmp->children[0];
            free(tmp->keys);
            free(tmp->records);
            free(tmp->children);
            free(tmp);
        }
    }
}

// untuk menu insert data mahasiswa ke dalam B-Tree
void menu_insert() {
   char name[52], major[52], id_input[11];
    float gpa;

    printf("Input name [5-50]: ");
    scanf(" %[^\n]", name);
    while (strlen(name) < 5 || strlen(name) > 50) {
        printf("Invalid name. Try again: ");
        scanf(" %[^\n]", name);
    }

    printf("Input major [5-50]: ");
    scanf(" %[^\n]", major);
    while (strlen(major) < 5 || strlen(major) > 50) {
        printf("Invalid major. Try again: ");
        scanf(" %[^\n]", major);
    }

    printf("Input GPA [0.00-4.00]: ");
    scanf("%f", &gpa);
    while (gpa < 0.00 || gpa > 4.00) {
        printf("Invalid GPA. Try again: ");
        scanf("%f", &gpa);
    }

    printf("Input ID [max 10 chars]: ");
    scanf(" %[^\n]", id_input);
    while (strlen(id_input) >= 11 || search_student(studentTree, id_input)) {
        printf("ID invalid or exists. Try again: ");
        scanf(" %[^\n]", id_input);
    }

    insert_student(studentTree, id_input, name, major, gpa);
    printf("Insert successful!\n");
    getchar();
}

// ini untuk view
void menu_view() {
    printf("\n-- All Students --\n");
    if (!studentTree->root) {
        printf("No records.\n");
    } else {
        printf("%-10s | %-20s | %-20s | %s\n", "ID", "Name", "Major", "GPA");
        printf("--------------------------------------------------------------\n");
        traverse(studentTree->root);
    }
    getchar(); getchar();
}

// ini untuk search
void menu_search() {
    char id_to_search[11];
    printf("Enter student ID to search: ");
    scanf(" %[^\n]", id_to_search);

    StudentRecord* result = search_student(studentTree, id_to_search);
    if (result) {
        printf("Student Found:\n");
        printf("%-10s | %-20s | %-20s | %s\n", "ID", "Name", "Major", "GPA");
        printf("%-10s | %-20s | %-20s | %.2f\n",
               result->studentID,
               result->name,
               result->major,
               result->gpa);
    } else {
        printf("Student ID %s not found.\n", id_to_search);
    }

    getchar(); getchar();
}

// ini untuk delete
void menu_delete() {
    char id[11];
    printf("Enter student ID to delete: ");
    scanf(" %10[^\n]", id);
    if (!search_student(studentTree, id)) {
        printf("ID not found.\n");
    } else {
        deleteKey(studentTree, id);
        printf("Deleted!\n");
    }
}

// ini main menu untuk menjalankan program
int main() {
    studentTree = createBTree(MIN_DEGREE, 10);
    int choice;
    do {
        printf("==============================\n");
        printf("= BEE University Student =\n");
        printf("==============================\n");
        printf("1. Insert Student\n");
        printf("2. View Records\n");
        printf("3. Search by ID\n");
        printf("4. Delete by ID\n");
        printf("5. Exit\n");
        printf(">>: ");
        scanf("%d", &choice);
        switch (choice) {
            case 1: menu_insert(); break;
            case 2: menu_view(); break;
            case 3: menu_search(); break;
            case 4: menu_delete(); break;
            case 5: printf("Bye!\n"); break;
            default: printf("Invalid\n");
        }
    } while (choice != 5);
    return 0;
}

