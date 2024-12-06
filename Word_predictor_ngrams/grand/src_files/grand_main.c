#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../header_files/btree.h"
#include "../header_files/functions.h"

 // Declare word_processor



int main() {
    // Step 1: Initialize the trie and load unigrams
    trie T;
    init_trie(&T);
    process_csv_file("./dataset/unigrams_4000.csv", &T);

    // Step 2: Initialize and load bigram and trigram B+ trees
    BPlusTree* bi_B_plus_tree = createBPlusTree();
    readCSVAndInsert(bi_B_plus_tree, "./dataset/bigrams_2000.csv");

    BPlusTree* tri_B_plus_tree = createBPlusTree();
    readCSVAndInsert(tri_B_plus_tree, "./dataset/trigrams_1000.csv");

    // Step 3: Get user input
    char user_string[500];
    printf("\n[DEBUG] Enter a string for prediction: ");
    if (fgets(user_string, sizeof(user_string), stdin) != NULL) {
        size_t len = strlen(user_string);
        if (len > 0 && user_string[len - 1] == '\n') {
            user_string[len - 1] = '\0'; // Remove newline character
        }
    }
    // printf("[DEBUG] User input received: %s\n", user_string);

    // Step 4: Tokenize user input and initialize stacks
    priority_Q res1, res2;
    init_priority_Q(&res1);
    init_priority_Q(&res2);
    stack s, processed_stack;
    init_stack(&s);
    init_stack(&processed_stack);
    tokenize_user_string(user_string, &s);

    // Step 5: Extract the last two words for prediction
    char *input_word1 = NULL, *input_word2 = NULL;
    int word_count = 0;

    for (int i = 0; i < 2; i++) {
        char *temp = NULL;
        while ((temp = pop(&s)) != NULL) {
            printf("[DEBUG] Popped token: %s\n", temp);
            if (is_word(temp)) {
                if (i == 0) {
                    input_word2 = temp;
                    word_count++;
                    // printf("[DEBUG] First word for prediction: %s\n", input_word2);
                } else {
                    input_word1 = temp;
                    word_count++;
                    // printf("[DEBUG] Second word for prediction: %s\n", input_word1);
                }
                break;
            }
            free(temp);
        }
    }

    // Step 6: Process remaining words and get the corrected string
    // printf("[DEBUG] Processing remaining words and correcting string...\n");
    char *corrected_string = word_processor(&s, &processed_stack, &T);
    // printf("[DEBUG] Corrected string: %s\n", corrected_string);

    word_element search_set[2];
    backoff(T, input_word1, input_word2, search_set,&res1, &res2);

    printf("[DEBUG] Search Set contains %d words:\n", word_count);
    for (int i = 0; i < 2; i++) {
        printf("[DEBUG] search_set[%d]: %s\n", i, search_set[i].word);
    }
    
    bt_priority_q bt_q;
    init_bt_pq(&bt_q);

    // // Step 7: Perform word prediction with backoff
    if (word_count == 2) {
        printf("[DEBUG] Two words found. Starting trigram search...\n");
        searchNGrams(tri_B_plus_tree, search_set[0].word, search_set[1].word, &bt_q);

        if (bt_q.top > 0) {
            printf("[DEBUG] Trigram search successful. Displaying results...\n");
            display_unique_bt_q(&bt_q, corrected_string);
        } else {
            printf("[DEBUG] No trigrams found. Backing off to bigram search...\n");
            //but here concatenate search_set[0] to the corrected string
            strcat(corrected_string, search_set[0].word);
            strcat(corrected_string, " ");
            searchNGrams(bi_B_plus_tree, search_set[1].word, NULL, &bt_q);

            if (bt_q.top > 0) {
                printf("[DEBUG] Bigram search successful. Displaying results...\n");
                display_unique_bt_q(&bt_q, corrected_string);
            } else {
                printf("[DEBUG] No suggestions found. Returning the corrected string.\n");
                printf("Final Context: %s%s %s\n", corrected_string, input_word1, input_word2);
            }
        }
    } else if (word_count == 1) {
        printf("[DEBUG] One word found. Starting bigram search...\n");
        searchNGrams(bi_B_plus_tree, search_set[1].word, NULL, &bt_q);

        if (bt_q.top > 0) {
            printf("[DEBUG] Bigram search successful. Displaying results...\n");
            display_unique_bt_q(&bt_q, corrected_string);
        } else {
            printf("[DEBUG] No suggestions found. Concatenating corrected string.\n");
            printf("Final Context: %s%s\n", corrected_string, input_word2);
        }
    } else {
        printf("[DEBUG] No valid words for prediction. Returning the corrected string.\n");
        printf("Final Context: %s\n", corrected_string);
    }

    // Cleanup
    free_bt_q(&bt_q);
    free(corrected_string);
    if (input_word1) free(input_word1);
    if (input_word2) free(input_word2);

    return 0;
}
