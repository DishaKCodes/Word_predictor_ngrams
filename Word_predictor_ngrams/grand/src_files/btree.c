#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../header_files/btree.h"


void init_bt_pq(bt_priority_q *bt_q) {
    // Set all n-grams to NULL and their counts to 0
    for (int i = 0; i < 3; i++) {
        bt_q->ngram[i] = NULL;
        bt_q->count[i] = 0;
    }
    bt_q->top = -1; // Indicates that the queue is initially empty
}

// Sort the priority queue by count in descending order
void sort_priority_queue(bt_priority_q *pq) {
    // Bubble sort algorithm to sort the queue
    for (int i = 0; i <= pq->top; i++) {
        for (int j = 0; j < pq->top - i; j++) {
            if (pq->count[j] < pq->count[j + 1]) { // Swap if counts are out of order
                // Swap counts
                int tempCount = pq->count[j];
                pq->count[j] = pq->count[j + 1];
                pq->count[j + 1] = tempCount;

                // Swap n-grams
                char *tempNgram = pq->ngram[j];
                pq->ngram[j] = pq->ngram[j + 1];
                pq->ngram[j + 1] = tempNgram;
            }
        }
    }
}

// Insert an n-gram with a count into the priority queue
void insert_bt_pq(bt_priority_q *bt_q, const char *ngram, int count) {
    if (bt_q->top < 2) {  // Allow up to 3 entries
        bt_q->top++;
        bt_q->ngram[bt_q->top] = strdup(ngram); // Add the n-gram to the queue
        bt_q->count[bt_q->top] = count;         // Add its count
        sort_priority_queue(bt_q);             // Keep the queue sorted
    } else if (count > bt_q->count[2]) { // If the count is higher than the smallest in the queue
        free(bt_q->ngram[2]);             // Remove the smallest n-gram
        bt_q->ngram[2] = strdup(ngram);  // Replace it with the new n-gram
        bt_q->count[2] = count;          // Update the count
        sort_priority_queue(bt_q);       // Re-sort the queue
    }
}

// Display the contents of the priority queue
void display_bt_q(bt_priority_q bt_q) {
    printf("Priority Queue Contents:\n");
    for (int i = 0; i <= bt_q.top; i++) {
        if (bt_q.ngram[i]) {
            // Print each n-gram and its count
            printf("N-gram: %s, Count: %d\n", bt_q.ngram[i], bt_q.count[i]);
        }
    }
}

// Free allocated memory in the priority queue
void free_bt_q(bt_priority_q *bt_q) {
    for (int i = 0; i <= bt_q->top; i++) {
        free(bt_q->ngram[i]);
    }
}

// Create and initialize a new B+ tree
BPlusTree* createBPlusTree() {
    BPlusTree *tree = (BPlusTree*)malloc(sizeof(BPlusTree));
    tree->root = NULL;
    tree->totalNgramsCount = 0;
    return tree;
}

// Create a new B+ tree node (leaf or internal)
BTreeNode* createNode(int isLeaf) {
    BTreeNode* node = (BTreeNode*)malloc(sizeof(BTreeNode));
    node->isLeaf = isLeaf; // Specify if this is a leaf node
    node->numKeys = 0;     // No keys initially
    node->next = NULL;     // No link to the next node yet
    for (int i = 0; i <= MAX_KEYS; i++) {
        node->children[i] = NULL; // Initialize child pointers to NULL
    }
    return node;
}

// Insert an n-gram into a leaf node
void insertIntoLeaf(BTreeNode* node, const char* ngram, int count) {
    int i;

    // Check if the n-gram already exists in the node
    for (i = 0; i < node->numKeys; i++) {
        if (strcmp(node->keys[i], ngram) == 0) {
            node->counts[i] += count; // If found, update its count
            return;
        }
    }

    // Insert the new n-gram in sorted order
    for (i = node->numKeys - 1; i >= 0 && strcmp(node->keys[i], ngram) > 0; i--) {
        strcpy(node->keys[i + 1], node->keys[i]); // Shift keys to make space
        node->counts[i + 1] = node->counts[i];
    }

    strcpy(node->keys[i + 1], ngram); // Insert the new n-gram
    node->counts[i + 1] = count;
    node->numKeys++; // Increase the key count
}

// function to Split a leaf node when it overflows
BTreeNode* splitLeaf(BTreeNode* leaf, const char* ngram, int count) {
    BTreeNode* newLeaf = createNode(1); // Create a new leaf node
    char tempKeys[MAX_KEYS + 1][200];   // Temporary array for keys
    int tempCounts[MAX_KEYS + 1];       // Temporary array for counts

    int i, j;

    // Copy existing keys and counts to temporary arrays
    for (i = 0; i < MAX_KEYS; i++) {
        strcpy(tempKeys[i], leaf->keys[i]);
        tempCounts[i] = leaf->counts[i];
    }

    // Insert the new n-gram into the temporary arrays
    for (i = MAX_KEYS - 1; i >= 0 && strcmp(tempKeys[i], ngram) > 0; i--) {
        strcpy(tempKeys[i + 1], tempKeys[i]);
        tempCounts[i + 1] = tempCounts[i];
    }
    strcpy(tempKeys[i + 1], ngram);
    tempCounts[i + 1] = count;

    // Redistribute keys and counts between the two leaf nodes
    //since it is a LEFT BIASED B+TREE tree
    int splitIndex = (MAX_KEYS + 1) / 2;
    leaf->numKeys = splitIndex;
    newLeaf->numKeys = MAX_KEYS + 1 - splitIndex;

    for (i = 0; i < leaf->numKeys; i++) {
        strcpy(leaf->keys[i], tempKeys[i]);
        leaf->counts[i] = tempCounts[i];
    }
    for (j = 0; j < newLeaf->numKeys; j++) {
        strcpy(newLeaf->keys[j], tempKeys[splitIndex + j]);
        newLeaf->counts[j] = tempCounts[splitIndex + j];
    }

    // Link the new leaf to the current one
    newLeaf->next = leaf->next;
    leaf->next = newLeaf;

    return newLeaf; // Return the new leaf node
}

BTreeNode* splitInternal(BTreeNode* node, int* promotedIndex) {
    BTreeNode* newInternal = createNode(0);
    int midIndex = MAX_KEYS / 2;
    *promotedIndex = midIndex;

    // Transfer the second half of keys and children to the new internal node
    newInternal->numKeys = MAX_KEYS - midIndex - 1;
    for (int i = 0; i < newInternal->numKeys; i++) {
        strcpy(newInternal->keys[i], node->keys[midIndex + 1 + i]);
        newInternal->children[i] = node->children[midIndex + 1 + i];
    }
    newInternal->children[newInternal->numKeys] = node->children[MAX_KEYS];
    node->numKeys = midIndex; // Update the original node's key count

    return newInternal;
}


BTreeNode* insertRecursive(BTreeNode* root, const char* ngram, int count, char* promotedKey, int* promotedCount, BTreeNode** newChild) {
    if (root->isLeaf) {
        // Insert the n-gram into the leaf node
        insertIntoLeaf(root, ngram, count);

        // Check if the leaf node exceeds the maximum allowed keys
        if (root->numKeys == MAX_KEYS) {
            // Split the leaf node and promote a key to the parent
            BTreeNode* newLeaf = splitLeaf(root, ngram, count);
            strcpy(promotedKey, newLeaf->keys[0]);
            *newChild = newLeaf;
            return NULL;
        }
    } else {
        // Traverse to the appropriate child node
        int i = 0;
        while (i < root->numKeys && strcmp(ngram, root->keys[i]) > 0) i++;

        // Recursively insert into the selected child
        BTreeNode* tempNewChild = NULL;
        char tempPromotedKey[200];
        BTreeNode* result = insertRecursive(root->children[i], ngram, count, tempPromotedKey, promotedCount, &tempNewChild);

        if (tempNewChild != NULL) {
            // If a child split occurred, adjust the current node
            for (int j = root->numKeys; j > i; j--) {
                strcpy(root->keys[j], root->keys[j - 1]);
                root->children[j + 1] = root->children[j];
            }
            strcpy(root->keys[i], tempPromotedKey);
            root->children[i + 1] = tempNewChild;
            root->numKeys++;

            // Check if the current node needs to be split
            if (root->numKeys == MAX_KEYS) {
                int promotedIndex;
                BTreeNode* newInternal = splitInternal(root, &promotedIndex);
                strcpy(promotedKey, root->keys[promotedIndex]);
                *newChild = newInternal;
                return NULL;
            }
        }
    }
    return root;
}

// Function to insert an n-gram into the B+ Tree
void insertBPlusTree(BPlusTree* tree, const char* ngram, int count) {
    if (tree->root == NULL) {
        // Create a root node if the tree is empty
        tree->root = createNode(1);
    }

    char promotedKey[200];
    int promotedCount = 0;
    BTreeNode* newChild = NULL;

    // Call the recursive insert function
    BTreeNode* newRoot = insertRecursive(tree->root, ngram, count, promotedKey, &promotedCount, &newChild);

    if (newChild != NULL) {
        // If the root was split, create a new root node
        newRoot = createNode(0);
        strcpy(newRoot->keys[0], promotedKey);
        newRoot->counts[0] = promotedCount;
        newRoot->children[0] = tree->root;
        newRoot->children[1] = newChild;
        newRoot->numKeys = 1;
        tree->root = newRoot;
    }

    // Update the total number of n-grams in the tree
    tree->totalNgramsCount += count;
}

// Function to read n-grams and counts from a CSV file and insert them into the B+ Tree
void readCSVAndInsert(BPlusTree* tree, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Could not open file %s\n", filename);
        return;
    }

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        // Parse the n-gram and count from the CSV line
        char* ngram = strtok(line, ",");
        char* countStr = strtok(NULL, "\n");
        if (ngram && countStr) {
            int count = atoi(countStr);
            insertBPlusTree(tree, ngram, count);
        }
    }

    fclose(file);
}

// Function to list all n-grams in the B+ Tree in ascending order
void listAllNgrams(BPlusTree* tree) {
    if (tree->root == NULL) return;

    // Navigate to the leftmost leaf node
    BTreeNode* current = tree->root;
    while (!current->isLeaf) {
        current = current->children[0];
    }

    // Traverse the leaf nodes and print their contents
    while (current != NULL) {
        for (int i = 0; i < current->numKeys; i++) {
            printf("%s: %d\n", current->keys[i], current->counts[i]);
        }
        current = current->next;  // Move to the next leaf node
    }
}

// Function to search for bigrams or trigrams in the B+ Tree
void searchNGrams(BPlusTree* tree, const char* firstWord, const char* secondWord, bt_priority_q* result) {
    if (tree->root == NULL) {
        printf("The tree is empty.\n");
        return;
    }

    BTreeNode* current = tree->root;

    // Navigate to the appropriate leaf node
    while (!current->isLeaf) {
        int i = 0;
        while (i < current->numKeys && strcmp(firstWord, current->keys[i]) > 0) {
            i++;
        }
        current = current->children[i];
    }

    // Search for matching n-grams
    while (current != NULL) {
        for (int i = 0; i < current->numKeys; i++) {
            char keyCopy[200];
            strcpy(keyCopy, current->keys[i]);

            if (secondWord == NULL) { // Bigram search
                if (strncmp(keyCopy, firstWord, strlen(firstWord)) == 0) {
                    insert_bt_pq(result, current->keys[i], current->counts[i]);
                }
            } else { // Trigram search
                char* token = strtok(keyCopy, " ");
                char first[200], second[200];

                if (token != NULL) {
                    strcpy(first, token);
                    token = strtok(NULL, " ");
                    if (token != NULL) {
                        strcpy(second, token);
                        if (strcmp(first, firstWord) == 0 && strcmp(second, secondWord) == 0) {
                            insert_bt_pq(result, current->keys[i], current->counts[i]);
                        }
                    }
                }
            }
        }
        current = current->next; // Move to the next leaf node
    }
}

// Function to display unique suggestions with their context
void display_unique_bt_q(bt_priority_q* bt_q, char* corrected_string) {
    printf("\n[DEBUG] Displaying suggestions with context:\n");
    int printed[MAX_KEYS] = {0};  // Array to track printed n-grams

    for (int i = 0; i < bt_q->top; i++) {
        if (!printed[i]) {
            // Print the suggestion with the corrected context
            printf("\n-----------------------------------------------------\n");
            printf("[DEBUG] Suggested n-gram: %s\n", bt_q->ngram[i]);
            printf("[DEBUG] Count: %d\n", bt_q->count[i]);
            printf("[DEBUG] Full Context: %s%s\n", corrected_string, bt_q->ngram[i]);
            printf("-----------------------------------------------------\n");

            // Mark duplicates as printed
            for (int j = i + 1; j < bt_q->top; j++) {
                if (strcmp(bt_q->ngram[i], bt_q->ngram[j]) == 0) {
                    printed[j] = 1;
                }
            }
        }
    }

    if (bt_q->top == 0) {
        printf("\n[DEBUG] No suggestions found.\n");
    }
}