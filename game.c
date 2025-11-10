#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "lab5.h"

extern Node *g_root;
extern EditStack g_undo;
extern EditStack g_redo;
extern Hash g_index;

/* TODO 31: Implement play_game
 * Main game loop using iterative traversal with a stack
 * 
 * Key requirements:
 * - Use FrameStack (NO recursion!)
 * - Push frames for each decision point
 * - Track parent and answer for learning
 * 
 * Steps:
 * 1. Initialize and display game UI
 * 2. Initialize FrameStack
 * 3. Push root frame with answeredYes = -1
 * 4. Set parent = NULL, parentAnswer = -1
 * 5. While stack not empty:
 *    a. Pop current frame
 *    b. If current node is a question:
 *       - Display question and get user's answer (y/n)
 *       - Set parent = current node
 *       - Set parentAnswer = answer
 *       - Push appropriate child (yes or no) onto stack
 *    c. If current node is a leaf (animal):
 *       - Ask "Is it a [animal]?"
 *       - If correct: celebrate and break
 *       - If wrong: LEARNING PHASE
 *         i. Get correct animal name from user
 *         ii. Get distinguishing question
 *         iii. Get answer for new animal (y/n for the question)
 *         iv. Create new question node and new animal node
 *         v. Link them: if newAnswer is yes, newQuestion->yes = newAnimal
 *         vi. Update parent pointer (or g_root if parent is NULL)
 *         vii. Create Edit record and push to g_undo
 *         viii. Clear g_redo stack
 *         ix. Update g_index with canonicalized question
 * 6. Free stack
 */
void play_game() {
    clear();
    attron(COLOR_PAIR(5) | A_BOLD);
    mvprintw(0, 0, "%-80s", " Playing 20 Questions");
    attroff(COLOR_PAIR(5) | A_BOLD);
    
    mvprintw(2, 2, "Think of an animal, and I'll try to guess it!");
    mvprintw(3, 2, "Press any key to start...");
    refresh();
    getch();
    
    // TODO: Implement the game loop
    // Initialize FrameStack
    // Push root
    // Loop until stack empty or guess is correct
    // Handle question nodes and leaf nodes differently
    
    // Initialize a FrameStack to perform iterative traversal
    FrameStack stack;
    fs_init(&stack);

    // Push the root node as the first frame; answeredYes = -1 means no parent answer
    fs_push(&stack, g_root, -1);

    // Track parent pointer and whether the current node is the parent's yes child
    Node *parent = NULL;
    int parentAnswer = -1; // 1 = yes child, 0 = no child

    // id is used to insert into the index hash; start from 0
    int id = 0;

    // Iterative traversal: continue until the stack is empty
    while (!fs_empty(&stack)) {
        // Pop the next frame to visit
        Frame curr = fs_pop(&stack);

        // Handle question nodes: prompt the user and push the chosen child
        if (curr.node->isQuestion) {
            // Clear the area and write to for a clean UI
            move(5, 0);
            clrtoeol();
            move(6, 0);
            clrtoeol();

            // Display the question text and prompt for yes/no
            mvprintw(5, 2, "%s", curr.node->text);
            mvprintw(6, 2, "Enter (y/n): ");
            refresh();

            // Read a single character answer (no echo)
            char ans = getch();

            // Remember the parent node for potential learning phase
            parent = curr.node;

            // If user answered yes, push the 'yes' child; otherwise push 'no'
            if (ans == 'Y' || ans == 'y') {
                fs_push(&stack, curr.node->yes, 1);
                parentAnswer = 1;
            } else {
                fs_push(&stack, curr.node->no, 0);
                parentAnswer = 0;
            }
        }

        // Handle leaf nodes (animals)
        if (!curr.node->isQuestion) {
            // Clear UI lines used for question/answer
            move(5, 0);
            clrtoeol();
            move(6, 0);
            clrtoeol();

            // Ask the user whether the guessed animal is correct
            mvprintw(5, 2, "Is it a %s?", curr.node->text);
            mvprintw(6, 2, "Enter (y/n): ");
            refresh();
            char ans = getch();

            if (ans == 'Y' || ans == 'y') {
                // Correct guess: show a confirmation and wait for key press
                move(5, 0);
                clrtoeol();
                move(6, 0);
                clrtoeol();
                mvprintw(5, 2, "I got the animal right!");
                mvprintw(6, 2, "Press any key to continue...");
                refresh();

                // Wait for the user to acknowledge
                getch();
                break; // game round ends
            } else {
                // Learning phase: ask user for the correct animal name
                char animalName[100];
                char question[500];

                move(5, 0);
                clrtoeol();
                move(6, 0);
                clrtoeol();
                mvprintw(5, 2, "I give up! What's your animal?");
                mvprintw(6, 2, "Name: ");
                refresh();

                // Enable echo to read a string line from the user
                echo();
                mvgetstr(6, 8, animalName); // read up to newline
                noecho();

                // Ask for the distinguishing question for the new animal
                move(8, 0);
                clrtoeol();
                move(9, 0);
                clrtoeol();
                mvprintw(8, 2, "What's your animal's distinguishing question?");
                mvprintw(9, 2, "Question: ");
                refresh();
                echo();
                mvgetstr(9, 12, question);
                noecho();

                // Ask for the correct answer to the new question for the new animal
                move(11, 0);
                clrtoeol();
                move(12, 0);
                clrtoeol();
                mvprintw(11, 2, "What's the answer to this quesiton? (y/n)");
                mvprintw(12, 2, "Answer: ");
                refresh();
                ans = getch();

                // Create new nodes: a question node and a leaf for the new animal
                Node *newQuestion = create_question_node(question);
                Node *newAnimal = create_animal_node(animalName);
                Node *oldAnimal = curr.node; // leaf we failed to guess

                // Link the new question node: place newAnimal on the branch
                // that corresponds to the user's answer to the distinguishing question
                if (ans == 'Y' || ans == 'y') {
                    newQuestion->yes = newAnimal;
                    newQuestion->no = oldAnimal;
                } else {
                    newQuestion->no = newAnimal;
                    newQuestion->yes = oldAnimal;
                }

                // Attach the newly-created question node to the parent in the tree
                if (parent == NULL) {
                    // We replaced the root directly
                    g_root = newQuestion;
                } else if (parentAnswer == 1) {
                    // Parent's yes pointer should point to the new question
                    parent->yes = newQuestion;
                } else {
                    // Parent's no pointer should point to the new question
                    parent->no = newQuestion;
                }

                // Create an Edit record so the change can be undone
                Edit newEdit;
                newEdit.type = EDIT_INSERT_SPLIT;
                newEdit.parent = parent;
                newEdit.oldLeaf = oldAnimal;
                newEdit.newQuestion = newQuestion;
                newEdit.newLeaf = newAnimal;
                newEdit.wasYesChild = parentAnswer;

                // Push the edit onto the undo stack and clear redo stack
                es_push(&g_undo, newEdit);
                es_clear(&g_redo);

                // Insert the canonicalized question into the index for searching
                char *canonicalizedQ = canonicalize(question);
                h_put(&g_index, canonicalizedQ, id++);
                free(canonicalizedQ);
            }

        }
    }

    // Free stack resources when done
    fs_free(&stack);
}

/* TODO 32: Implement undo_last_edit
 * Undo the most recent tree modification
 * 
 * Steps:
 * 1. Check if g_undo stack is empty, return 0 if so
 * 2. Pop edit from g_undo
 * 3. Restore the tree structure:
 *    - If edit.parent is NULL:
 *      - Set g_root = edit.oldLeaf
 *    - Else if edit.wasYesChild:
 *      - Set edit.parent->yes = edit.oldLeaf
 *    - Else:
 *      - Set edit.parent->no = edit.oldLeaf
 * 4. Push edit to g_redo stack
 * 5. Return 1
 * 
 * Note: We don't free newQuestion/newLeaf because they might be redone
 */
int undo_last_edit() {
    // If there are no edits to undo, return 0
    if (es_empty(&g_undo)) {
        return 0;
    }
    // Pop the most recent edit from the undo stack
    Edit curr = es_pop(&g_undo);
    // Restore the tree to the state before the edit by reconnecting the
    // parent's pointer (or the root) back to the old leaf node
    if (curr.parent == NULL) {
        // Edit changed the root; restore old leaf as root
        g_root = curr.oldLeaf;
    } else if (curr.wasYesChild) {
        // Parent's yes pointer should point back to the old leaf
        curr.parent->yes = curr.oldLeaf;
    } else {
        // Parent's no pointer should point back to the old leaf
        curr.parent->no = curr.oldLeaf;
    }
    // Push the undone edit onto the redo stack so it can be redone later
    es_push(&g_redo, curr);
    return 1;
}

/* TODO 33: Implement redo_last_edit
 * Redo a previously undone edit
 * 
 * Steps:
 * 1. Check if g_redo stack is empty, return 0 if so
 * 2. Pop edit from g_redo
 * 3. Reapply the tree modification:
 *    - If edit.parent is NULL:
 *      - Set g_root = edit.newQuestion
 *    - Else if edit.wasYesChild:
 *      - Set edit.parent->yes = edit.newQuestion
 *    - Else:
 *      - Set edit.parent->no = edit.newQuestion
 * 4. Push edit back to g_undo stack
 * 5. Return 1
 */
int redo_last_edit() {
    // If there is nothing to redo, return failure
    if (es_empty(&g_redo)) {
        return 0;
    }
    // Pop the most recent undone edit
    Edit curr = es_pop(&g_redo);
    // Re-apply the change: attach the new question node at the parent's spot
    if (curr.parent == NULL) {
        // The edit replaced the root, so restore newQuestion as root
        g_root = curr.newQuestion;
    //Same cases logic as undo
    } else if (curr.wasYesChild) { 
        curr.parent->yes = curr.newQuestion;
    } else {
        curr.parent->no = curr.newQuestion;
    }
    // Push the edit back onto the undo stack
    es_push(&g_undo, curr);
    return 1;
}
