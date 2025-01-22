/******************
Name: Gal Neeman
ID: 331981365
Assignment: ex5
*******************/
#include "ex6.h"

# define INT_BUFFER 128

// ================================================
// Basic struct definitions from ex6.h assumed:
//   PokemonData { int id; char *name; PokemonType TYPE; int hp; int attack; EvolutionStatus CAN_EVOLVE; }
//   PokemonNode { PokemonData* data; PokemonNode* left, *right; }
//   OwnerNode   { char* ownerName; PokemonNode* pokedexRoot; OwnerNode *next, *prev; }
//   OwnerNode* ownerHead;
//   const PokemonData pokedex[];
// ================================================

// --------------------------------------------------------------
// 1) Safe integer reading
// --------------------------------------------------------------

void trimWhitespace(char *str) {
    // Remove leading spaces/tabs/\r
    int start = 0;
    while (str[start] == ' ' || str[start] == '\t' || str[start] == '\r')
        start++;

    if (start > 0) {
        int idx = 0;
        while (str[start])
            str[idx++] = str[start++];
        str[idx] = '\0';
    }

    // Remove trailing spaces/tabs/\r
    int len = (int) strlen(str);
    while (len > 0 && (str[len - 1] == ' ' || str[len - 1] == '\t' || str[len - 1] == '\r')) {
        str[--len] = '\0';
    }
}

char *myStrdup(const char *src) {
    if (!src)
        return NULL;
    size_t len = strlen(src);
    char *dest = (char *) malloc(len + 1);
    if (!dest) {
        printf("Memory allocation failed in myStrdup.\n");
        return NULL;
    }
    strcpy(dest, src);
    return dest;
}

int readIntSafe(const char *prompt) {
    char buffer[INT_BUFFER];
    int value;
    int success = 0;

    while (!success) {
        printf("%s", prompt);
        // If we fail to read, treat it as invalid
        if (!fgets(buffer, sizeof(buffer), stdin)) {
            printf("Invalid input.\n");
            clearerr(stdin);
            continue;
        }

        // 1) Strip any trailing \r or \n
        //    so "123\r\n" becomes "123"
        size_t len = strlen(buffer);
        if (len > 0 && (buffer[len - 1] == '\n' || buffer[len - 1] == '\r'))
            buffer[--len] = '\0';
        if (len > 0 && (buffer[len - 1] == '\r' || buffer[len - 1] == '\n'))
            buffer[--len] = '\0';

        // 2) Check if empty after stripping
        if (len == 0) {
            printf("Invalid input.\n");
            continue;
        }

        // 3) Attempt to parse integer with strtol
        char *endptr;
        value = (int) strtol(buffer, &endptr, 10);

        // If endptr didn't point to the end => leftover chars => invalid
        // or if buffer was something non-numeric
        if (*endptr != '\0') {
            printf("Invalid input.\n");
        } else {
            // We got a valid integer
            success = 1;
        }
    }
    return value;
}

// --------------------------------------------------------------
// 2) Utility: Get type name from enum
// --------------------------------------------------------------
const char *getTypeName(PokemonType type) {
    switch (type) {
        case GRASS:
            return "GRASS";
        case FIRE:
            return "FIRE";
        case WATER:
            return "WATER";
        case BUG:
            return "BUG";
        case NORMAL:
            return "NORMAL";
        case POISON:
            return "POISON";
        case ELECTRIC:
            return "ELECTRIC";
        case GROUND:
            return "GROUND";
        case FAIRY:
            return "FAIRY";
        case FIGHTING:
            return "FIGHTING";
        case PSYCHIC:
            return "PSYCHIC";
        case ROCK:
            return "ROCK";
        case GHOST:
            return "GHOST";
        case DRAGON:
            return "DRAGON";
        case ICE:
            return "ICE";
        default:
            return "UNKNOWN";
    }
}

// --------------------------------------------------------------
// Utility: getDynamicInput (for reading a line into malloc'd memory)
// --------------------------------------------------------------
char *getDynamicInput() {
    char *input = NULL;
    size_t size = 0, capacity = 1;
    input = (char *) malloc(capacity);
    if (!input) {
        printf("Memory allocation failed.\n");
        return NULL;
    }

    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
        if (size + 1 >= capacity) {
            capacity *= 2;
            char *temp = (char *) realloc(input, capacity);
            if (!temp) {
                printf("Memory reallocation failed.\n");
                free(input);
                return NULL;
            }
            input = temp;
        }
        input[size++] = (char) c;
    }
    input[size] = '\0';

    // Trim any leading/trailing whitespace or carriage returns
    trimWhitespace(input);

    return input;
}

// Function to print a single Pokemon node
void printPokemonNode(PokemonNode *node) {
    if (!node)
        return;
    printf("ID: %d, Name: %s, Type: %s, HP: %d, Attack: %d, Can Evolve: %s\n",
           node->data->id,
           node->data->name,
           getTypeName(node->data->TYPE),
           node->data->hp,
           node->data->attack,
           (node->data->CAN_EVOLVE == CAN_EVOLVE) ? "Yes" : "No");
}

void freePokemonTree(PokemonNode *root) {
    if (root == NULL) return;
    free(root->data);
    freePokemonTree(root->left);
    freePokemonTree(root->right);
    free(root);
}

void freeOwnerNode(OwnerNode *owner) {
    free(owner->ownerName);
    freePokemonTree(owner->pokedexRoot);
    free(owner);
}

void freeAllOwners() {
    if (ownerHead == NULL) return;
    //adding a NULL at the end to know when to stop
    ownerHead->prev->next = NULL;
    OwnerNode *cur = ownerHead;
    while (cur != NULL) {
        OwnerNode *temp = cur;
        cur = cur->next;
        freeOwnerNode(temp);
    }
}

PokemonData *createPokemonData(int id) {
    PokemonData *data = (PokemonData *) malloc(sizeof(PokemonData));
    if (data == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }
    *data = pokedex[id - 1];
    return data;
}

PokemonNode *createPokemonNode(int id) {
    PokemonData *data = createPokemonData(id);
    PokemonNode *pokNode = (PokemonNode *) malloc(sizeof(PokemonNode));
    if (pokNode == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }
    pokNode->data = data;
    pokNode->left = NULL;
    pokNode->right = NULL;
    return pokNode;
}

OwnerNode *createOwner(char *ownerName, PokemonNode *starter) {
    OwnerNode *node = (OwnerNode *) malloc(sizeof(OwnerNode));
    if (node == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }
    node->ownerName = ownerName;
    node->pokedexRoot = starter;
    return node;
}

//a function to collect the pokemons into an array in level order
void collectAll(PokemonNode *root, PokemonNode *arr[AMOUNT_OF_POKEMONS], int *size) {
    arr[0] = root;
    *size = 1;
    int counter = 0;
    while (counter < *size) {
        if (arr[counter]->left != NULL) {
            (*size)++;
            arr[*size - 1] = arr[counter]->left;
        }
        if (arr[counter]->right != NULL) {
            (*size)++;
            arr[*size - 1] = arr[counter]->right;
        }
        counter++;
    }
}

void BFSGeneric(PokemonNode *root, VisitNodeFunc visit) {
    PokemonNode *arr[AMOUNT_OF_POKEMONS];
    int size;
    collectAll(root, arr, &size);
    for (int i = 0; i < size; i++) visit(arr[i]);
}

void preOrderGeneric(PokemonNode *root, VisitNodeFunc visit) {
    if (root == NULL) return;
    visit(root);
    preOrderGeneric(root->left, visit);
    preOrderGeneric(root->right, visit);
}

void inOrderGeneric(PokemonNode *root, VisitNodeFunc visit) {
    if (root == NULL) return;
    inOrderGeneric(root->left, visit);
    visit(root);
    inOrderGeneric(root->right, visit);
}

void postOrderGeneric(PokemonNode *root, VisitNodeFunc visit) {
    if (root == NULL) return;
    postOrderGeneric(root->left, visit);
    postOrderGeneric(root->right, visit);
    visit(root);
}


void displayAlphabetical(PokemonNode *root) {
    int size;
    PokemonNode *arr[AMOUNT_OF_POKEMONS];
    collectAll(root, arr, &size);
    //bubble sort
    for (int i = 0; i < size-1; i++) {
        for (int j = 0; j < size-i-1; j++) {
            if (strcmp(arr[j]->data->name, arr[j+1]->data->name) > 0) {
                //swap
                PokemonNode *temp = arr[j];
                arr[j] = arr[j+1];
                arr[j+1] = temp;
            }
        }
    }
    for (int i = 0; i < size; i++) printPokemonNode(arr[i]);
}


// --------------------------------------------------------------
// Display Menu
// --------------------------------------------------------------
void displayMenu(OwnerNode *owner) {
    if (!owner->pokedexRoot) {
        printf("Pokedex is empty.\n");
        return;
    }

    printf("Display:\n");
    printf("1. BFS (Level-Order)\n");
    printf("2. Pre-Order\n");
    printf("3. In-Order\n");
    printf("4. Post-Order\n");
    printf("5. Alphabetical (by name)\n");

    int choice = readIntSafe("Your choice: ");

    switch (choice) {
        case 1:
            BFSGeneric(owner->pokedexRoot, printPokemonNode);
            break;
        case 2:
            preOrderGeneric(owner->pokedexRoot, printPokemonNode);
            break;
        case 3:
            inOrderGeneric(owner->pokedexRoot, printPokemonNode);
            break;
        case 4:
            postOrderGeneric(owner->pokedexRoot, printPokemonNode);
            break;
        case 5:
            displayAlphabetical(owner->pokedexRoot);
            break;
        default:
            printf("Invalid choice.\n");
    }
}


OwnerNode *findOwnerByName(const char *name) {
    if (ownerHead == NULL) return NULL;
    OwnerNode *cur = ownerHead;
    do {
        if (strcmp(name, cur->ownerName) == 0) return cur;
        cur = cur->next;
    } while (cur != ownerHead);
    return NULL;
}

void insertPokemonNode(PokemonNode *root, PokemonNode *newNode) {
    if (root->data->id > newNode->data->id) {
        if (root->left == NULL) root->left = newNode;
        else insertPokemonNode(root->left, newNode);
    } else {
        if (root->right == NULL) root->right = newNode;
        else insertPokemonNode(root->right, newNode);
    }
}

PokemonNode *searchPokemonBFS(PokemonNode *root, int id) {
    if (root == NULL) return NULL;
    if (root->data->id > id) return searchPokemonBFS(root->left, id);
    if (root->data->id < id) return searchPokemonBFS(root->right, id);
    return root;
}

PokemonNode *removeNodeBST(PokemonNode *root) {
    free(root->data);
    if (root->left == NULL) {
        PokemonNode *temp = root->right;
        free(root);
        return temp;
    }
    if (root->right == NULL) {
        PokemonNode *temp = root->left;
        free(root);
        return temp;
    }
    //replacing the root with the smallest id larger than it
    PokemonNode *cur = root->right, *prev = NULL;
    while (cur->left != NULL) {
        prev = cur;
        cur = cur->left;
    }
    root->data = cur->data;
    if (prev == NULL) root->right = cur->right;
    else prev->left = cur->right;
    free(cur);
    return root;
}

PokemonNode *removePokemonByID(PokemonNode *root, int id) {
    if (root->data->id > id) {
        root->left = removePokemonByID(root->left, id);
        return root;
    }
    if (root->data->id < id) {
        root->right = removePokemonByID(root->right, id);
        return root;
    }
    return removeNodeBST(root);
}

void evolvePokemon(OwnerNode *owner) {
    if (owner->pokedexRoot == NULL) {
        printf("Cannot evolve. Pokedex empty.\n");
        return;
    }
    int id = readIntSafe("Enter ID of Pokemon to evolve: ");
    PokemonNode *cur = searchPokemonBFS(owner->pokedexRoot, id);
    if (cur == NULL) {
        printf("No Pokemon with ID %d found.\n", id);
        return;
    }
    if (!cur->data->CAN_EVOLVE) {
        printf("Pokemon %s (ID %d) can't evolve.\n", cur->data->name, id);
        return;
    }
    //if the evolved version is already in the tree just remove the node
    if (searchPokemonBFS(owner->pokedexRoot, id + 1) != NULL)
        owner->pokedexRoot =
                removePokemonByID(owner->pokedexRoot, id);
    else cur->data = createPokemonData(id + 1);
    printf("Pokemon evolved from %s (ID %d) to %s (ID %d).\n", pokedex[id - 1].name, pokedex[id - 1].id,
           pokedex[id].name, pokedex[id].id);
}

void pokemonFight(OwnerNode *owner) {
    if (owner->pokedexRoot == NULL) {
        printf("Pokedex is empty.\n");
        return;
    }
    PokemonData pok1 = pokedex[readIntSafe("Enter ID of the first Pokemon: ") - 1],
            pok2 = pokedex[readIntSafe("Enter ID of the second Pokemon: ")-1], winner = pok1;
    if (searchPokemonBFS(owner->pokedexRoot, pok1.id) && searchPokemonBFS(owner->pokedexRoot, pok2.id)) {
        double pok1Score = pok1.attack * 1.5 + pok1.hp * 1.2, pok2Score = pok2.attack * 1.5 + pok2.hp * 1.2;
        printf("Pokemon 1: %s (Score = %.2f)\nPokemon 2: %s (Score = %.2f)\n", pok1.name, pok1Score,
            pok2.name, pok2Score);
        if (pok1Score == pok2Score) {
            printf("It’s a tie!\n");
            return;
        }
        if (pok1Score < pok2Score) winner = pok2;
        printf("%s wins!\n", winner.name);
    } else printf("One or both Pokemon IDs not found.\n");
}

void freePokemon(OwnerNode *owner) {
    if (owner->pokedexRoot == NULL) {
        printf("No Pokemon to release.\n");
        return;
    }
    int id = readIntSafe("Enter Pokemon ID to release: ");
    if (searchPokemonBFS(owner->pokedexRoot, id) == NULL) {
        printf("No Pokemon with ID %d found.\n", id);
        return;
    }
    owner->pokedexRoot = removePokemonByID(owner->pokedexRoot, id);
    printf("Removing Pokemon %s (ID %d).", pokedex[id - 1].name, id);
}

void addPokemon(OwnerNode *owner) {
    int id = readIntSafe("Enter ID to add: ");
    if (owner->pokedexRoot == NULL) {
        owner->pokedexRoot = createPokemonNode(id);
        printf("Pokemon %s (ID %d) added.", pokedex[id - 1].name, id);
        return;
    }
    if (searchPokemonBFS(owner->pokedexRoot, id) != NULL) {
        printf("Pokemon with ID %d is already in the Pokedex. No changes made.", id);
        return;
    }
    insertPokemonNode(owner->pokedexRoot, createPokemonNode(id));
    printf("Pokemon %s (ID %d) added.", pokedex[id - 1].name, id);
}

void printOwnerList(int *size) {
    OwnerNode *cur = ownerHead;
    *size = 1;
    do {
        printf("%d. %s\n", *size, cur->ownerName);
        cur = cur->next;
        (*size)++;
    } while (cur != ownerHead);
}

void mergeTrees(PokemonNode *firstTree, PokemonNode *secondTree) {
    if (secondTree == NULL) return;
    if (searchPokemonBFS(firstTree, secondTree->data->id) == NULL)
        insertPokemonNode(firstTree, createPokemonNode(secondTree->data->id));
    mergeTrees(firstTree, secondTree->left);
    mergeTrees(firstTree, secondTree->right);
}

void removeOwnerFromCircularList(OwnerNode *target) {
    freePokemonTree(target->pokedexRoot);
    free(target->ownerName);
    target->prev->next = target->next;
    target->next->prev = target->prev;
    //update ownerhead if it got removed
    if (ownerHead == target) ownerHead = target->next;
    free(target);
}

int getAmountOfOwners() {
    OwnerNode *cur = ownerHead;
    int size = 0;
    do {
        cur = cur->next;
        size++;
    } while (cur != ownerHead);
    return size;
}

void swapOwnerData(OwnerNode *a, OwnerNode *b) {
    char *tempName = a->ownerName;
    PokemonNode *tempPokemon = a->pokedexRoot;
    a->ownerName = b->ownerName;
    a->pokedexRoot = b->pokedexRoot;
    b->ownerName = tempName;
    b->pokedexRoot = tempPokemon;
}

void printOwnersCircular() {
    if (ownerHead == NULL) {
        printf("No owners.\n");
        return;
    }
    printf("Enter direction (F or B): ");
    char direction = getDynamicInput()[0];
    int times = readIntSafe("How many prints? ");
    OwnerNode *cur = ownerHead;
    for (int i = 0; i < times; i++) {
        printf("[%d] %s\n", i + 1, cur->ownerName);
        if (tolower(direction) == 'b') cur = cur->prev;
        else cur = cur->next;
    }
}

void sortOwners() {
    if (ownerHead == NULL) {
        printf("0 or 1 owners only => no need to sort.\n");
        return;
    }
    int size = getAmountOfOwners();
    for (int i = 0; i < size - 1; i++) {
        OwnerNode *cur = ownerHead;
        for (int j = 0; j < size - i - 1; j++) {
            if (strcmp(cur->ownerName, cur->next->ownerName) > 0) {
                swapOwnerData(cur, cur->next);
            }
            cur = cur->next;
        }
    }
    printf("Owners sorted by name.\n");
}

void mergePokedexMenu() {
    if (ownerHead == NULL || ownerHead == ownerHead->next) {
        printf("Not enough owners to merge.\n");
        return;
    }
    printf("\n=== Merge Pokedexes ===\n");
    printf("Enter name of first owner: ");
    OwnerNode *firstOwner = findOwnerByName(getDynamicInput());
    printf("Enter name of second owner: ");
    char *secondOwnerName = getDynamicInput();
    OwnerNode *secondOwner = findOwnerByName(secondOwnerName);
    printf("Merging %s and %s...\n", firstOwner->ownerName, secondOwner->ownerName);
    if (secondOwner->pokedexRoot != NULL && firstOwner->pokedexRoot != NULL)
        mergeTrees(firstOwner->pokedexRoot, secondOwner->pokedexRoot);
    else if (firstOwner->pokedexRoot == NULL) firstOwner->pokedexRoot = secondOwner->pokedexRoot;
    printf("Merge completed.\nOwner '%s' has been removed after merging.", secondOwner->ownerName);
    removeOwnerFromCircularList(secondOwner);
}


void deletePokedex() {
    if (ownerHead == NULL) {
        printf("No existing Pokedexes to delete.\n");
        return;
    }
    printf("\n=== Delete a Pokedex ===\n");
    int size;
    printOwnerList(&size);
    int choice = readIntSafe("Choose a Pokedex to delete by number: ");
    if (choice >= size || choice <= 0) {
        printf("Invalid choice.\n");
        return;
    }
    OwnerNode *cur = ownerHead;
    for (int i = 0; i < choice - 1; i++) cur = cur->next;
    printf("Deleting %s's entire Pokedex...\n", cur->ownerName);
    removeOwnerFromCircularList(cur);
    printf("Pokedex deleted.\n");
}

// --------------------------------------------------------------
// Sub-menu for existing Pokedex
// --------------------------------------------------------------
void enterExistingPokedexMenu() {
    if (ownerHead == NULL) {
        printf("No existing Pokedexes.\n");
        return;
    }
    printf("\nExisting Pokedexes:\n");
    int size;
    printOwnerList(&size);
    int choice = readIntSafe("Choose a Pokedex by number: ");
    if (choice >= size || choice <= 0) {
        printf("Invalid choice.\n");
        return;
    }
    OwnerNode *cur = ownerHead;
    for (int i = 0; i < choice - 1; i++) cur = cur->next;
    printf("\nEntering %s's Pokedex...\n", cur->ownerName);

    int subChoice;
    do {
        printf("\n-- %s's Pokedex Menu --\n", cur->ownerName);
        printf("1. Add Pokemon\n");
        printf("2. Display Pokedex\n");
        printf("3. Release Pokemon (by ID)\n");
        printf("4. Pokemon Fight!\n");
        printf("5. Evolve Pokemon\n");
        printf("6. Back to Main\n");

        subChoice = readIntSafe("Your choice: ");


        switch (subChoice) {
            case 1:
                addPokemon(cur);
                break;
            case 2:
                displayMenu(cur);
                break;
            case 3:
                freePokemon(cur);
                break;
            case 4:
                pokemonFight(cur);
                break;
            case 5:
                evolvePokemon(cur);
                break;
            case 6:
                printf("Back to Main Menu.\n");
                break;
            default:
                printf("Invalid choice.\n");
        }
    } while (subChoice != 6);
}


void openPokedexMenu() {
    printf("Your name: ");
    char *name = getDynamicInput();
    if (findOwnerByName(name) != NULL) {
        printf("Owner '%s' already exists. Not creating a new Pokedex.", name);
        return;
    }
    int starterChoice = readIntSafe("Choose Starter:\n1. Bulbasaur\n2. Charmander\n3. Squirtle\nYour choice: ");
    if (starterChoice > 3 || starterChoice < 1) {
        printf("Invalid choice.\n");
        return;
    }
    //3*starterchoice-2 is the matching id to the choice
    PokemonNode *pokNode = createPokemonNode(3 * starterChoice - 2);
    OwnerNode *ownerNode = createOwner(name, pokNode);
    if (ownerHead == NULL) {
        ownerHead = ownerNode;
        ownerNode->next = ownerHead;
        ownerNode->prev = ownerHead;
    } else {
        ownerNode->next = ownerHead;
        ownerNode->prev = ownerHead->prev;
        ownerNode->prev->next = ownerNode;
        ownerHead->prev = ownerNode;
    }
    printf("New Pokedex created for %s with starter %s.", name, pokNode->data->name);
}


// --------------------------------------------------------------
// Main Menu
// --------------------------------------------------------------
void mainMenu() {
    int choice;
    do {
        printf("\n=== Main Menu ===\n");
        printf("1. New Pokedex\n");
        printf("2. Existing Pokedex\n");
        printf("3. Delete a Pokedex\n");
        printf("4. Merge Pokedexes\n");
        printf("5. Sort Owners by Name\n");
        printf("6. Print Owners in a direction X times\n");
        printf("7. Exit\n");
        choice = readIntSafe("Your choice: ");

        switch (choice) {
            case 1:
                openPokedexMenu();
                break;
            case 2:
                enterExistingPokedexMenu();
                break;
            case 3:
                deletePokedex();
                break;
            case 4:
                mergePokedexMenu();
                break;
            case 5:
                sortOwners();
                break;
            case 6:
                printOwnersCircular();
                break;
            case 7:
                printf("Goodbye!\n");
                break;
            default:
                printf("Invalid.\n");
        }
    } while (choice != 7);
}

int main() {
    mainMenu();
    freeAllOwners();
    return 0;
}
