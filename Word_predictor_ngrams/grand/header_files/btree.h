#ifndef BTREE_H
#define BTREE_H

#define MAX_KEYS 200
#define MAX_LINE_LENGTH 300

// The B+ Tree implementation is a LEFT-BIASED B+ TREE.
// This means that when splitting nodes, keys are retained in the left node as much as possible.

// Priority queue structure to store the top three n-grams with their counts.
typedef struct {
    char *ngram[3];     // Array to store the top three n-grams.
    int count[3];       // Array to store the counts corresponding to the n-grams.
    int top;            // Tracks the current number of entries in the priority queue.
} bt_priority_q;

// Structure for a B+ Tree node.
// Internal nodes are used for navigation, and leaf nodes store the actual data (n-grams and their counts).
typedef struct BTreeNode {
    int isLeaf;                           // Flag to indicate if the node is a leaf (1) or internal (0).
    int numKeys;                          // Current number of keys in the node.
    char keys[MAX_KEYS][200];             // Array of keys stored in the node.
    int counts[MAX_KEYS];                 // Array of counts (used only in leaf nodes).
    struct BTreeNode *children[MAX_KEYS + 1];// Array of pointers to child nodes (internal nodes only).
    struct BTreeNode *next;               // Pointer to the next leaf node (used for leaf node chaining).
} BTreeNode;

// Structure for the B+ Tree itself.
typedef struct BPlusTree {
    BTreeNode *root;                      // Pointer to the root node of the tree.
    long long int totalNgramsCount;       // Total count of n-grams stored in the tree.
} BPlusTree;

// Initializes the priority queue used for storing n-grams.
void init_bt_pq(bt_priority_q *bt_q);

// Sorts the priority queue to ensure the top three n-grams are ranked by count.
void sort_priority_queue(bt_priority_q *pq);

// Inserts an n-gram along with its count into the priority queue.
void insert_bt_pq(bt_priority_q *bt_q, const char *ngram, int count);

// Debugging function to display the contents of the priority queue.
void display_bt_q(bt_priority_q bt_q);

// Frees any resources allocated for the priority queue.
void free_bt_q(bt_priority_q *bt_q);

// Initializes a new B+ Tree and returns a pointer to it.
BPlusTree* createBPlusTree();

// Creates a new B+ Tree node (either internal or leaf) based on the given flag.
BTreeNode* createNode(int isLeaf);

// Inserts a new n-gram into a leaf node. If the leaf is full, a split will be triggered.
void insertIntoLeaf(BTreeNode* node, const char* ngram, int count);

// Splits a full leaf node into two leaf nodes and handles key redistribution.
BTreeNode* splitLeaf(BTreeNode* leaf, const char* word, int count);

// Splits a full internal node into two internal nodes and handles key redistribution. Promotes the middle key to the parent node.
BTreeNode* splitInternal(BTreeNode* node, int* promotedIndex);

// Recursive helper function for inserting n-grams into the B+ Tree. Manages splits at both leaf and internal node levels.
BTreeNode* insertRecursive(BTreeNode* root, const char* ngram, int count, char* promotedKey, int* promotedCount, BTreeNode** newChild);

// Inserts a new n-gram into the B+ Tree. Handles root splits and ensures tree properties are maintained.
void insertBPlusTree(BPlusTree* tree, const char* ngram, int count);

// Reads n-grams from a CSV file and inserts them into the B+ Tree.
void readCSVAndInsert(BPlusTree* tree, const char* filename);

// Traverses the B+ Tree and lists all n-grams along with their counts.
void listAllNgrams(BPlusTree* tree);

// Searches for n-grams in the B+ Tree based on a given prefix. Results are stored in a priority queue.
void searchNGrams(BPlusTree* tree, const char* firstWord, const char* secondWord, bt_priority_q *result);


//function to display the result along with the corrected string
void display_unique_bt_q(bt_priority_q *bt_q, char *corrected_string);


#endif // BTREE_H

