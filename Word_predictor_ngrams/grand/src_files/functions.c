#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include "../header_files/functions.h"

// Initialize the priority queue to an empty state
void init_priority_Q(priority_Q * pq) {
    pq->size = 0;
    return;
}

// Initialize the trie structure for storing words
void init_trie(trie * T) {
    T->root = (trie_node *)malloc(sizeof(trie_node));                                           // Create root node
    T->root->count = 0;                                                                         // Initialize root node count
    T->root->isEndOfWord = false;                                                               // Root is not the end of any word initially
    for (int i = 0; i < 26; i++) {
        T->root->children[i] = NULL;                                                            // Initialize all child pointers to NULL
    }
    T->total_unigram_count = 0;
}

// Create a new trie node
trie_node * get_node() {
    trie_node * nn = (trie_node *)malloc(sizeof(trie_node));
    if (nn) {
        for (int i = 0; i < 26; i++) {
            nn->children[i] = NULL;                                                             // Initialize all child pointers to NULL
        }
        nn->count = 0;                                                                          // Initialize count to zero
        nn->isEndOfWord = false;                     
    }
    return nn;
}

// Insert a word into the trie with its occurrence count
void insert_word(trie * T, const char * word, int count) {
    trie_node * p = T->root;
    int index;
    for (int i = 0; word[i] != '\0'; i++) {
        index = word[i] - 'a';                                                                  // Map character to index
        if (p->children[index] == NULL) {
            p->children[index] = get_node();                                                    // Create a new node if none exists
        }
        p = p->children[index];                                                                 // Move to the next level
    }
    p->isEndOfWord = true;                                                                      // Mark the end of a word
    p->count += count;                                                                          // Update the word's count
    T->total_unigram_count += count;                                                            // Increment the total word count
}

// Process a CSV file to populate the trie with words and their counts
//**
void process_csv_file(const char *filename, trie *T) {
   
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Could not open file");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char word[128];
        int count;


        if (sscanf(line, "%[^,],%d", word, &count) == 2) {                                   // Parse the line to extract the word and count
            insert_word(T, word, count);                                                     // Insert the word into the trie
        }
    }

    fclose(file);
}

// Calculate unigram probability (occurrence count / total word's count in corpus)
double unigram_prob(int count, int total) {
    return (double)count / (double)total;
}

// Check if a token exists as a unigram in the trie and add it to the priority queue if found
double is_unigram(trie T, const char *token, priority_Q *pq) {
    trie_node *p = T.root;
    int i = 0, index;
    while (token[i] != '\0') {
        index = token[i] - 'a';
        if (p->children[index] == NULL) {                                                   // If the path doesn't exist, return 0
            return 0.0;
        }
        p = p->children[index];                                                             // Move to the next character
        i++;
    }
    if (p != NULL && p->isEndOfWord) {                                                      // If the word exists in the trie
        word_element result;
        strcpy(result.word, token);
        result.prob = unigram_prob(p->count, T.total_unigram_count);
        result.distance = 0;
        insert_pq(pq, result.word, result.prob, result.distance);                           // Add to priority queue
        return result.prob;
    }
    return 0.0;                                                                             // Token not found
}

// Check if a token is a prefix of any word in the trie
int is_prefix(trie T, const char * token, priority_Q * result) {
    trie_node * p = T.root;
    int i = 0, index;
    while (token[i] != '\0') {
        index = token[i] - 'a';
        if (p->children[index] == NULL) {                                                   // If path doesn't exist, return 0
            return 0;
        }
        p = p->children[index];                                                             // Move to the next character
        i++;
    }
    char word[100];
    strcpy(word, token);                                                                    // Start collecting words with this prefix
    collect_words(p, result, word, strlen(token), T.total_unigram_count);
    return (result->size > 0);                                                              // Return 1 if any words found
}

// Collect all words from the current node and its children in the trie
void collect_words(trie_node * curr, priority_Q * result, char * word, int level, long long int total_unigram_count) {
    if (curr == NULL) {
        return;
    }
    if (curr->isEndOfWord) {                                                                // If it's the end of a word, add it
        word[level] = '\0';
        insert_pq(result, word, unigram_prob(curr->count, total_unigram_count), 0.0);
    }
    for (int i = 0; i < 26; i++) {                                                          //otherwise scan all its children pointers to collect words starting with the given prefix
        if (curr->children[i] != NULL) {
            word[level] = (char)('a' + i);                                                  // Add the current character
            collect_words(curr->children[i], result, word, level + 1, total_unigram_count);
        }
    }
}

// Insert an element into the priority queue while keeping it sorted by probability
void insert_pq(priority_Q *result, const char *word, double prob, float dist) {
    if (result->size < MAX_WORDS) {                                                         //check if the priority queue is not full
        strcpy(result->words_collection[result->size].word, word);
        result->words_collection[result->size].prob = prob;
        result->words_collection[result->size].distance = dist;
        result->size++;

        // sort the priority queue : using BUBBLE SORT
        for (int i = result->size - 1; i > 0; i--) {
            if (result->words_collection[i].prob > result->words_collection[i - 1].prob) {
                word_element temp = result->words_collection[i];
                result->words_collection[i] = result->words_collection[i - 1];
                result->words_collection[i - 1] = temp;
            } else {
                break;
            }
        }
    } else {
        // Replace the element with minimum probability if the new probability is larger
        int minIndex = 0;
        for (int i = 1; i < MAX_WORDS; i++) {
            if (result->words_collection[i].prob < result->words_collection[minIndex].prob) {
                minIndex = i;
            }
        }
        if (prob > result->words_collection[minIndex].prob) {
            strcpy(result->words_collection[minIndex].word, word);
            result->words_collection[minIndex].prob = prob;
            result->words_collection[minIndex].distance = dist;

            // Re-sort after replacing the smallest element
            for (int i = minIndex; i > 0; i--) {
                if (result->words_collection[i].prob > result->words_collection[i - 1].prob) {
                    word_element temp = result->words_collection[i];
                    result->words_collection[i] = result->words_collection[i - 1];
                    result->words_collection[i - 1] = temp;
                } else {
                    break;
                }
            }
        }
    }
}

// Find the minimum of three values
int min3(int a, int b, int c) {
    if (a <= b && a <= c){
        return a;
    }
    if (b <= a && b <= c){
        return b;
    }
    return c;
}

//****************************************************************************
// Calculate the normalized edit distance (Levenshtein ratio) between two words
//time complexity is n * m
//space complexity is also n * m
float edit_distance(const char* word1, const char* word2) {
    int len1 = strlen(word1);
    int len2 = strlen(word2);

    // Allocate memory for the edit distance matrix
    int ** matrix = (int **)malloc(sizeof(int*) * (len1 + 1));
    for (int i = 0; i < len1 + 1; i++) {
        matrix[i] = (int *)malloc(sizeof(int) * (len2 + 1));
    }

    // Initialize the matrix
    for (int i = 0; i <= len1; i++){
        matrix[i][0] = i;
    }
    for (int j = 0; j <= len2; j++){
        matrix[0][j] = j;
    }

    for (int i = 1; i <= len1; i++) {
        for (int j = 1; j <= len2; j++) {
            if (word1[i - 1] == word2[j - 1]) {
                matrix[i][j] = matrix[i - 1][j - 1];
            } else {
                matrix[i][j] = min3(matrix[i - 1][j], matrix[i][j - 1], matrix[i - 1][j - 1]) + 1;
            }
        }
    }

    int raw_edit_distance = matrix[len1][len2];

    for (int i = 0; i <= len1; i++){
        free(matrix[i]);
    }
    free(matrix);

    float average_len = (float)(len1 + len2) / 2.0;
    float edit_distance_ratio = (float)raw_edit_distance / average_len;                  // Normalize the distance
    return edit_distance_ratio;
}



// void sort_by_edits(priority_Q *pq) {
//     for (int i = 0; i < pq->size - 1; i++) {
//         for (int j = 0; j < pq->size - i - 1; j++) {
//             if (pq->words_collection[j].distance > pq->words_collection[j + 1].distance) {
//                 word_element temp = pq->words_collection[j];
//                 pq->words_collection[j] = pq->words_collection[j + 1];
//                 pq->words_collection[j + 1] = temp; // Swap elements
//             }
//         }
//     }
// }

void collect_fuzzy(trie_node * p, priority_Q *pq, const char * token, char * curr_word, int level, long long int total_unigram_count){
    if(p == NULL){
        return;
    }
    if(p -> isEndOfWord){
        curr_word[level] = '\0';
        float edits = edit_distance(token, curr_word);
        if(edits <= MAX_EDIT_DISTANCE){
            insert_pq(pq, curr_word, unigram_prob(p -> count, total_unigram_count), edits);
        }
    }
    for(int i = 0; i < 26; i++){
        if(p->children[i] != NULL){
            curr_word[level] = 'a' + i;
            collect_fuzzy(p->children[i], pq, token, curr_word, level + 1, total_unigram_count);
        }
    }
}

int is_fuzzymatch(trie T, const char * token, priority_Q * result){
    trie_node * p = T.root;
    char word[100] = "";
    collect_fuzzy(p, result, token, word, 0, T.total_unigram_count);
    if (result->size == 0) {
        return 0;
    }
    return 1;
}


void backoff(trie T, const char *token1, const char *token2, word_element search_set[], priority_Q *result1, priority_Q *result2) {
    word_element empty = {"", 0.0, 0};

    if (token1 != NULL) {
        word_element result1_word = validate(T, token1, result1);
        if (result1_word.prob != 0.0) {
            search_set[0] = result1_word;
        } else {
            search_set[0] = empty;
        }
    } else {
        search_set[0] = empty;
    }

    if (token2 != NULL) {
        word_element result2_word = validate(T, token2, result2);
        if (result2_word.prob != 0.0) {
            search_set[1] = result2_word;
        } else {
            search_set[1] = empty;
        }
    } else {
        search_set[1] = empty;
    }
}


word_element validate(trie T, const char * token, priority_Q * result){
    printf("*********************\n");
    if(is_unigram(T, token, result)){
        return result -> words_collection[0];
    }else if(is_prefix(T, token, result)){
        return result -> words_collection[0];
    }else if(is_fuzzymatch(T, token, result)){
        return result -> words_collection[0];
    }
    word_element empty = {"", 0.0, 0};
    return empty;
}

// Helper function to recursively print all words in the trie
void display_trie_helper(trie_node *node, char *prefix, int level) {
    if (node == NULL){
        return;                                                                         // Base case: node is NULL
    }
                                                      
    if (node->isEndOfWord) {                                                            // If the node marks the end of a word
        prefix[level] = '\0';
        printf("%s -> %d\n", prefix, node->count);                                      // Print the word and its count
    }
    // Recurse for all children
    for (int i = 0; i < 26; i++) {
        if (node->children[i] != NULL) {
            prefix[level] = 'a' + i;                                                    // Add the current character to the prefix
            display_trie_helper(node->children[i], prefix, level + 1);
        }
    }
}

// Print all words stored in the trie
void display_trie(trie T) {
    char prefix[100];// Buffer for building words
    display_trie_helper(T.root, prefix, 0);
}

// Initialize a stack
void init_stack(stack *s) {
    s->top = NULL;
    s->size = 0;
}

// Push a token onto the stack
void push(stack *s, char *token) {
    node *nn = (node *)malloc(sizeof(node));
    if (nn == NULL) {
        printf("Memory allocation failed!\n");
        return;
    }
    nn->next = s->top;
    strcpy(nn->token, token);
    s->top = nn;
    s->size++;
}

// Check if the stack is empty
int is_empty(stack s) {
    return s.top == NULL;
}

// Check if a token is a valid word (not a number or hashtag)
int is_word(const char *token) {
    if (isdigit(token[0]) || token[0] == '#') {
        return 0;
    }
    return 1;
}

//pop the top token from the stack
char *pop(stack *s) {
    if (is_empty(*s)) {
        printf("The stack is empty; no valid tokens to pop.\n");
        return NULL;
    }
    node *temp = s->top;                    
    s->top = s->top->next;                  
    char *token = strdup(temp->token);      
    free(temp);
    s->size--;
    return token;                           
}

// Convert a string to lowercase
void to_lower(char *input_string) {
    for (int i = 0; input_string[i] != '\0'; i++) {
        input_string[i] = tolower(input_string[i]);
    }
}

// Check if a character is a delimiter
int is_delim(char ch) {
    return strchr(DELIMS, ch) != NULL;
}

// Tokenize a user string into a stack of tokens
//**
void tokenize_user_string(char *user_string, stack *s) {
    char buffer[MAX_TOKEN_LEN];                                                         // Buffer for tokens
    int curr = 0;
    char ch;

    for (int i = 0; user_string[i] != '\0'; i++) {
        ch = user_string[i];
        if (is_delim(ch) || isspace(ch)) {                                              // If the character is a delimiter or space
            if (curr != 0) {                                                            // If the buffer contains a token
                buffer[curr] = '\0';
                to_lower(buffer);                                                       // Convert the token to lowercase
                push(s, buffer);                                                        // Push the token onto the stack
                curr = 0;
            }
            continue;                                                                   //otherwise skip over the delimiter
        }

        if (ch == '#') {                                                                // Handle hashtags
            buffer[curr++] = ch;
            i++;
            if (isalnum(user_string[i])) {
                while (isalnum(user_string[i])) {
                    if (curr < MAX_TOKEN_LEN - 1) {
                        buffer[curr++] = user_string[i];
                    }
                    i++;
                }
                i--;
                buffer[curr] = '\0';
                push(s, buffer);
                curr = 0;
                continue;
            } else {
                curr = 0;                                                               // Reset if invalid hashtag
                continue;
            }
        }
        if ((ch & 0xF0) == 0xF0) {                                                      // Handle emojis
            i += 3;                                                                     // Skip emoji bytes
            continue;
        }

        if (isdigit(ch)) {                                                              // Handle numbers
            while (isdigit(user_string[i]) || user_string[i] == '.') {                  //handle decimal numbers as well
                if (curr < MAX_TOKEN_LEN - 1) {
                    buffer[curr++] = user_string[i];
                }
                i++;
            }
            i--;
            continue;
        }

        if (curr < MAX_TOKEN_LEN - 1) {                                                 // Add valid character to the buffer
            buffer[curr++] = ch;
        }
    }

    if (curr != 0) {                                                                    // Push the last token onto the stack
        buffer[curr] = '\0';
        to_lower(buffer);
        push(s, buffer);
    }
}

// Process the stack of tokens, correct them, and concatenate into a string
char* word_processor(stack *s1, stack *s2, trie *T) {
    char *token;
    priority_Q pq;
    char *result = (char*)malloc(MAX_TOKEN_LEN * 100 * sizeof(char));                   // Allocate memory for the result string
    if (result == NULL) {
        printf("Memory allocation failed!\n");
        return NULL;
    }
    result[0] = '\0';                                                                   // Initialize result string

// Process each token in the input stack
    while (!is_empty(*s1)) {
        token = pop(s1);
        init_priority_Q(&pq);

        to_lower(token);

        if (!is_word(token)) {
            push(s2, token);
            free(token);
            continue;
        }

        
        double unigram_result = is_unigram(*T, token, &pq);                             // Try unigram matching

        if (unigram_result == 0.0) {
                                                                                        
            int prefix_result = is_prefix(*T, token, &pq);                              // Try prefix matching if unigram fails

            if (prefix_result == 0) {
                
                is_fuzzymatch(*T, token, &pq);                                          // Try fuzzy matching if prefix matching also fails
            }
        }

        if (pq.size > 0) {                                                              // If suggestions exist, use the best match
            push(s2, pq.words_collection[0].word);
        } else {                                                                        // Otherwise, keep the original token
            push(s2, token);
        }

        free(token);                                                                    // Free the original token
    }

    // Concatenate tokens from the second stack into the result string
    while (!is_empty(*s2)) {
        token = pop(s2);
        strcat(result, token);
        strcat(result, " ");                                                            // Add space between tokens
        free(token);
    }

    return result; // Return the corrected string
}




