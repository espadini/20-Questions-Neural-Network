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
    
    FrameStack stack;
    fs_init(&stack);

    fs_push(&stack, g_root, -1);
    Node* parent = NULL;
    int parentAnswer = -1;
    int id = 0;

    while(!(fs_empty(&stack))){
        Frame curr = fs_pop(&stack);
        if(curr.node->isQuestion) {
            move(5, 0); // Move cursor to row 5, col 0
            clrtoeol(); // Clear to end of line
            move(6, 0); // Move cursor to row 6, col 0
            clrtoeol(); // Clear to end of line

            // Display the question (at row 5, col 2)
            mvprintw(5, 2, "%s", curr.node->text);
            // Display the prompt (at row 6, col 2)
            mvprintw(6, 2, "Enter (y/n): ");
            refresh(); // Show the changes
            char ans = getch();

            parent = curr.node;

            if (ans == 'Y' || ans == 'y') {
                fs_push(&stack, curr.node->yes, 1);
                parentAnswer = 1;
            }
            else {
                fs_push(&stack, curr.node->no, 0);
                parentAnswer = 0;
            }
        }
        if(!(curr.node->isQuestion)) {
            move(5, 0); // Move cursor to row 5, col 0
            clrtoeol(); // Clear to end of line
            move(6, 0); // Move cursor to row 6, col 0
            clrtoeol(); // Clear to end of line

            // Display the question (at row 5, col 2)
            mvprintw(5, 2, "Is it a %s?", curr.node->text);
            // Display the prompt (at row 6, col 2)
            mvprintw(6, 2, "Enter (y/n): ");
            refresh(); // Show the changes
            char ans = getch();


            if (ans == 'Y' || ans == 'y') {
                move(5, 0); // Move cursor to row 5, col 0
                clrtoeol(); // Clear to end of line
                move(6, 0); // Move cursor to row 6, col 0
                clrtoeol(); // Clear to end of line
                mvprintw(5, 2, "I got the animal right!");
                mvprintw(6, 2, "Press any key to continue..."); 
                refresh();
                
                getch(); // <-- ADD THIS LINE to wait for input
                break;
            }
            else {
                char animalName[100];
                char question[500];

                move(5, 0); // Move cursor to row 5, col 0
                clrtoeol(); // Clear to end of line
                move(6, 0); // Move cursor to row 6, col 0
                clrtoeol(); // Clear to end of line
                mvprintw(5, 2, "I give up! What's your animal?");
                mvprintw(6, 2, "Name: ");
                refresh();
                echo();
                mvgetstr(6, 8, animalName);
                noecho();

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

                move(11, 0);
                clrtoeol();
                move(12, 0);
                clrtoeol();
                mvprintw(11, 2, "What's the answer to this quesiton? (y/n)");
                mvprintw(12, 2, "Answer: ");
                refresh();
                ans = getch();

                // iv. Create new question node and new animal node
                Node* newQuestion = create_question_node(question);
                Node* newAnimal = create_animal_node(animalName);
                Node* oldAnimal = curr.node; // This is the node we're replacing
                
                // v. Link them
                if (ans == 'Y' || ans == 'y') {
                    newQuestion->yes = newAnimal;
                    newQuestion->no = oldAnimal;
                } else {
                    newQuestion->no = newAnimal;
                    newQuestion->yes = oldAnimal;
                }

                if(parent == NULL) {g_root = newQuestion;}
                else if(parentAnswer == 1) {parent->yes = newQuestion;}
                else {parent->no = newQuestion;}

                Edit newEdit;
                newEdit.type = EDIT_INSERT_SPLIT; 
                newEdit.parent = parent;        
                newEdit.oldLeaf = oldAnimal;   
                newEdit.newQuestion = newQuestion;
                newEdit.newLeaf = newAnimal;
                newEdit.wasYesChild = parentAnswer;

                es_push(&g_undo, newEdit);
                es_clear(&g_redo);
                
                char* canonicalizedQ = canonicalize(question);
                h_put(&g_index, canonicalizedQ, id++);
                free(canonicalizedQ);
            }

        }
    }
    
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
    // TODO: Implement this function
    if(es_empty(&g_undo)) {return 0;}
    Edit curr = es_pop(&g_undo);

    if(curr.parent == NULL) {g_root = curr.oldLeaf;}
    else if(curr.wasYesChild) {curr.parent->yes = curr.oldLeaf;}
    else {curr.parent->no = curr.oldLeaf;}

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
    // TODO: Implement this function
    if(es_empty(&g_redo)) {return 0;}
    Edit curr = es_pop(&g_redo);

    if(curr.parent == NULL) {g_root = curr.newQuestion;}
    else if(curr.wasYesChild) {curr.parent->yes = curr.newQuestion;}
    else {curr.parent->no = curr.newQuestion;}

    es_push(&g_undo, curr);
    return 1;
}
