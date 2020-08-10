#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

/* Represents an individual position on the board */
typedef struct {
    int row;
    int column;
    int score;
    char stone;
} Position;

void check_arguments(char pOType, char pXType, FILE* saveFile);
void read_savefile(FILE* saveFile, char pOType, char pXType);
void assign_positions(Position* positions, char** board, int rows, 
	int columns);
void check_full_board(int rows, int columns, Position* positions);
void check_game_setup(int rows, int columns, int numRows, 
	char** separatedRows);
void check_board(int rows, int columns, int numRows, char** separatedRows);
bool correct_corner_position(char** separatedRows, int rows, int columns);
void update_board(char** board, Position* positions, int rows, int columns);
void display_board(char** board, int rows, int columns);
void play_game(char** board, int rows, int columns, Position* positions, 
	char pOType, char pXType, char* currentPlayer);
void automated_o_move(int rows, int columns, Position* positions, char pOType, 
	char* currentPlayer);
void automated_x_move(int rows, int columns, Position* positions, char pXType, 
	char* currentPlayer);
int* type1(Position* positions, int rows, int columns, 
	char* currentPlayer);
void human_o_move(char** board, int rows, int columns, Position* positions, 
	char* currentPlayer);
void human_x_move(char** board, int rows, int columns, Position* positions, 
	char* currentPlayer);
char** check_savefile(char* buffer);
bool valid_position(int chosenRow, int chosenColumn, int rows, int columns, 
	Position* positions);
bool outer_position(int chosenRow, int chosenColumn, int rows, int columns);
bool valid_push(int chosenRow, int chosenColumn, int rows, int columns, 
	Position* positions);
int* get_o_score(Position* positions, int rows, int columns);
int* get_x_score(Position* positions, int rows, int columns);
bool decrease_score(Position* positions, int chosenRow, int chosenColumn, 
	int rows, int columns, char* currentPlayer);
void push_stones(int chosenRow, int chosenColumn, int rows, int columns, 
	Position* positions, char* currentPlayer);
void downward_push(Position* positions, int rows, int columns, 
	int chosenIndex, char* currentPlayer);
void left_push(Position* positions, int rows, int columns, int chosenIndex, 
	char* currentPlayer);
void right_push(Position* positions, int rows, int columns, int chosenIndex, 
	char* currentPlayer);
void upward_push(Position* positions, int rows, int columns, int chosenIndex, 
	char* currentPlayer);
bool game_over(Position* positions, int rows, int columns);
void display_winners(Position* positions, int rows, int columns);
void save_game(char** board, int rows, int columns, char* fileName, char* 
	currentPlayer);

int main(int argc, char** argv) {
    /* Check the number of arguments */
    if (argc != 4) {
        fprintf(stderr, "Usage: push2310 typeO typeX fname\n");
        exit(1);
    }

    char pOType, pXType;
    pOType = argv[1][0];
    pXType = argv[2][0];
    FILE* saveFile = fopen(argv[3], "r");

    check_arguments(pOType, pXType, saveFile);
    
    read_savefile(saveFile, pOType, pXType);

    return 0;
}

/* Ensures that the arguments given to the program
 * Will exit if player types are invalid or if there was an error
 * opening the specified save file */
void check_arguments(char pOType, char pXType, FILE* saveFile) {
    /* Check player O type */
    if (pOType != '0' && pOType != '1' && pOType != 'H') {
        fprintf(stderr, "Invalid player type\n");
        exit(2);
    }

    /* Check player X type */
    if (pXType != '0' && pXType != '1' && pXType != 'H') {
        fprintf(stderr, "Invalid player type\n");
        exit(2);
    }

    if (saveFile == 0) {
        fprintf(stderr, "No file to load from\n");
        exit(3);
    }
}

/* Read the board into an array of positions, each of which has
 * a row, column, score and stone
 * Return the array of positions
 * */
Position* initialise_positions(int rows, int columns, char** board) {
    Position* positions = malloc(sizeof(Position) * rows * columns);
    int j = 0, r, c;

    for (r = 0; r < rows; r++) {
        for (c = 0; c < columns * 2; c += 2) {
            Position position;
            position.row = r;
            position.column = c / 2;
            
            if (board[r][c] == ' ') {
                position.score = 0;
            } else {
                position.score = board[r][c] - '0';
            }

            position.stone = board[r][c + 1];
            positions[j] = position;
            j++;
        }
    }

    return positions;
}

/* Read the contents of the savefile and initialise a board
 * made up of the savefile contents for game play*/
void read_savefile(FILE* saveFile, char pOType, char pXType) {
    fseek(saveFile, 0, SEEK_END);
    long length = ftell(saveFile);
    fseek(saveFile, 0, SEEK_SET);

    int i = 0, rows, columns;
    char* currentPlayer = (char*)malloc(sizeof(char));
    long offset = 0;
    char* buffer = (char*)malloc(sizeof(char) * length + 1);
    
    if (buffer == NULL) {
        fprintf(stderr, "Invalid file contents\n");
        exit(4);
    }

    while (!feof(saveFile) && offset < length) {
        offset += fread(buffer + offset, sizeof(char), length - offset, 
		saveFile);
    }
    buffer[offset] = '\0';
    
    fclose(saveFile);

    char** separatedRows = check_savefile(buffer);
    sscanf(separatedRows[0], "%d %d", &rows, &columns);
    currentPlayer = separatedRows[1];
    for (i = 2; i < rows + 2; i++) {
        separatedRows[i][strlen(separatedRows[i])] = '\n';
    }

    /* Read the savefile into a 2d array for the board */
    char** board = (char**)malloc(sizeof(char*) * rows);
    int r, c;
    for (r = 0; r < rows; r++) {
        board[r] = malloc(sizeof(char) * columns * 2 + 1);

        for (c = 0; c < columns * 2 + 1; c++) {
            board[r][c] = separatedRows[r + 2][c];
        }
    } 

    Position* positions = initialise_positions(rows, columns, board);

    check_full_board(rows, columns, positions); 
    display_board(board, rows, columns);
    play_game(board, rows, columns, positions, pOType, pXType, currentPlayer);
}

/* Ensure that the board read from the savefile isn't full
 * A board is full if there are stones in all the interior positions
 * Exit if the board is full
 * */
void check_full_board(int rows, int columns, Position* positions) {
    int i;

    for (i = 0; i < rows * columns; i++) {
        if (i == rows * columns - 1) {
            fprintf(stderr, "Full board in load\n");
            exit(6);
        }

        if (outer_position(positions[i].row, positions[i].column, rows,
                columns)) {
            continue;
        }

        if (positions[i].stone == '.') {
            break;
        }
    }
}

/* Ensure that the contents in the savefile are valid
 * Exit if the current player type is invalid
 * Exit if the number of rows and columns are invalid or incorrect
 * Return an array of the rows of the board on success
 * */
char** check_savefile(char* buffer) {
    int rows, columns, numRows = 0;
    char** separatedRows = NULL;
    char* position = strtok(buffer, "\n");

    /* Check that the number of rows and columns are valid */
    if (sscanf(buffer, "%d %d", &rows, &columns) != 2) {
        fprintf(stderr, "Invalid file contents\n");
        exit(4);
    }

    /* Separate each row of the board to check their contents */
    while (position) {
        separatedRows = realloc(separatedRows, sizeof(char*) * ++numRows);
        if (separatedRows == NULL) {
            fprintf(stderr, "Invalid file contents\n");
        }
        separatedRows[numRows - 1] = position;
        position = strtok(NULL, "\n");
    }
    
    separatedRows = realloc(separatedRows, sizeof(char*) * (numRows + 1));
    separatedRows[numRows] = 0;

    check_game_setup(rows, columns, numRows, separatedRows);
    check_board(rows, columns, numRows, separatedRows);

    return separatedRows;
}

/* Ensure that the board specified in the savefile is correct and valid
 * Exit if the stones are invalid or if the scores are invalid
 * */
void check_board(int rows, int columns, int numRows, char** separatedRows) {
    int i, j;

    /* Check the positions in the interior rows of the board */
    for (i = 3; i < numRows - 1; i++) {
        for (j = 0; j < columns * 2; j += 2) {
            if (!isdigit(separatedRows[i][j])) {
                fprintf(stderr, "Invalid file contents\n");
                exit(4);
            }
            if (separatedRows[i][j + 1] != '.' && separatedRows[i][j + 1] !=
                    'X' && separatedRows[i][j + 1] != 'O') {
                fprintf(stderr, "Invalid file contents\n");
                exit(4);
            }
        }
    }

    /* Check the positions in the first and last row */
    for (i = 2; i < numRows; i += (rows + 1)) {
        for (j = 2; j < columns * 2 - 2; j += 2) {
            if (!isdigit(separatedRows[i][j])) {
                fprintf(stderr, "Invalid file contents\n");
                exit(4);
            }
            if (separatedRows[i][j + 1] != '.' && separatedRows[i][j + 1] !=
                    'X' && separatedRows[i][j + 1] != 'O') {
                fprintf(stderr, "Invalid file contents\n");
                exit(4);
            }
        }
    } 
}

/* Ensure that the savefile contents are correct and valid
 * Check the specified current player, the the number of rows and columns
 * are correct and that all of the corner positions of the board are empty
 * */
void check_game_setup(int rows, int columns, int numRows, 
	char** separatedRows) {
    int i;

    /* Check the current player type */
    char currentPlayer = separatedRows[1][0];
    if (strlen(separatedRows[1]) != 1 || (currentPlayer != 'O' && 
	    currentPlayer != 'X')) {
        fprintf(stderr, "Invalid file contents\n");
        exit(4);
    }

    /* Check that the number of rows is correct */
    if (numRows != rows + 2) {
        fprintf(stderr, "Invalid file contents\n");
        exit(4);
    }

    /* Check that the number of columns is correct */
    for (i = 2; i < numRows; i++) {
        if (strlen(separatedRows[i]) != columns * 2) {
            fprintf(stderr, "Invalid file contents\n");
            exit(4);
        }
    }

    /* Check board contents*/
    if (!correct_corner_position(separatedRows, rows, columns)) {
        fprintf(stderr, "Invalid file contents\n");
        exit(4);
    }
}

/* Ensure that the corner positions of the board are empty
 * Return true if the corner positions are empty, or false otherwise
 * */
bool correct_corner_position(char** separatedRows, int rows, int columns) {
    if (separatedRows[2][0] == ' ') {
	return true;
    } else if (separatedRows[2][1] == ' ') {
	return true;
    } else if (separatedRows[2][columns * 2 - 2] == ' ') {
	return true;
    } else if (separatedRows[2][columns * 2 - 1] == ' ') {
	return true;
    } else if (separatedRows[rows + 1][0] == ' ') {
	return true;
    } else if (separatedRows[rows + 1][1] == ' ') {
	return true;
    } else if (separatedRows[rows + 1][columns * 2 - 2] == ' ') {
	return true;
    } else if (separatedRows[rows + 1][columns * 2 - 1] == ' ') {
	return true;
    } else {
	return false;
    }
}

/* Initiate game play
 * Prompt for the correct move type depending on player types
 * Update and display the board after each move
 * Print the winner of the game once the game is over
 * */
void play_game(char** board, int rows, int columns, Position* positions, 
	char pOType, char pXType, char* currentPlayer) {
    while (!game_over(positions, rows, columns)) {
        if (*currentPlayer == 'O' && pOType == 'H') {
            human_o_move(board, rows, columns, positions, currentPlayer);
        } else if (*currentPlayer == 'X' && pXType == 'H') {
            human_x_move(board, rows, columns, positions, currentPlayer);
        } else if (*currentPlayer == 'O' && pOType != 'H') {
            automated_o_move(rows, columns, positions, pOType, currentPlayer);
	} else {
            automated_x_move(rows, columns, positions, pXType, currentPlayer);
        }

        update_board(board, positions, rows, columns);
        display_board(board, rows, columns);       
    }

    display_winners(positions, rows, columns);            
}

/* Update the board once a move has been made
 * */
void update_board(char** board, Position* positions, int rows, int columns) {
    int i;

    for (i = 0; i < rows * columns; i++) {
        if (positions[i].stone == ' ') {
            board[positions[i].row][positions[i].column * 2] = ' ';
        } else {
            board[positions[i].row][positions[i].column * 2] = 
		    positions[i].score + '0';
        }

        board[positions[i].row][positions[i].column * 2 + 1] = 
	        positions[i].stone;
    }
}

/* Print the current board
 * */
void display_board(char** board, int rows, int columns) {
    int r, c;

    for (r = 0; r < rows; r++) {
        for (c = 0; c < columns * 2 + 1; c++) {
            printf("%c", board[r][c]);
        }
    }
}

/* Carry out a move for player O when they are an automated player
 * */
void automated_o_move(int rows, int columns, Position* positions, char pOType, 
	char* currentPlayer) {
    int chosenRow, chosenColumn;

    if (pOType == '0') {
        chosenRow = 1;
        chosenColumn = 1;
        while (!valid_position(chosenRow, chosenColumn, rows, columns, 
		positions)) {
            if (chosenColumn == columns - 2) {
                chosenColumn = 1;
                chosenRow++;
            } else {
                chosenColumn++;
            }
        }

        int i;
        for (i = 0; i < rows * columns; i++) {
            if (positions[i].row == chosenRow && positions[i].column == 
		    chosenColumn) {
                positions[i].stone = *currentPlayer;
            }
        }
    } else {
        int* chosenPosition = (int*)malloc(sizeof(int) * 2);
        chosenPosition = type1(positions, rows, columns, currentPlayer);
        chosenRow = chosenPosition[0];
        chosenColumn = chosenPosition[1];

        if (outer_position(chosenRow, chosenColumn, rows, columns)) {
            push_stones(chosenRow, chosenColumn, rows, columns, positions, 
		    currentPlayer);
        } else {
            int i;
            for (i = 0; i < rows * columns; i++) {
                if (positions[i].row == chosenRow && positions[i].column == 
			chosenColumn) {
                    positions[i].stone = *currentPlayer;
                }
            }
        }
    }
    printf("Player %c placed at %d %d\n", *currentPlayer, chosenRow, 
	    chosenColumn);

    *currentPlayer = 'X';
}

/* Carry out a move for player X when they are an automated player
 * */
void automated_x_move(int rows, int columns, Position* positions, char pXType, 
	char* currentPlayer) {
    int chosenRow = rows - 2, chosenColumn = columns - 2;

    if (pXType == '0') {
        while (!valid_position(chosenRow, chosenColumn, rows, columns, 
		positions)) {
            if (chosenColumn == 1) {
                chosenColumn = columns - 2;
                chosenRow--;
            } else {
                chosenColumn--;
            }
        }
 
        int i;
        for (i = 0; i < rows * columns; i++) {
            if (positions[i].row == chosenRow && positions[i].column == 
		    chosenColumn) {
                positions[i].stone = *currentPlayer;
            }
        }
    } else {
        int* chosenPosition = (int*)malloc(sizeof(int) * 2);
        chosenPosition = type1(positions, rows, columns, currentPlayer);
        chosenRow = chosenPosition[0];
        chosenColumn = chosenPosition[1];

        if (outer_position(chosenRow, chosenColumn, rows, columns)) {
            push_stones(chosenRow, chosenColumn, rows, columns, positions, 
		    currentPlayer);
        } else {
            int i;
            for (i = 0; i < rows * columns; i++) {
                if (positions[i].row == chosenRow && positions[i].column == 
			chosenColumn) {
                    positions[i].stone = *currentPlayer;
                }
            }
        }
    }
    printf("Player %c placed at %d %d\n", *currentPlayer, chosenRow, 
	    chosenColumn);

    *currentPlayer = 'O';
}

/* Find a valid type 1 move for automated players
 * Return an array of the coordinates of the position to be played
 * */
int* type1(Position* positions, int rows, int columns, char* currentPlayer) {
    int i, *chosenPosition = (int*)malloc(sizeof(int) * 2);
    for (i = 1; i < columns - 1; i++) {
        if (valid_position(0, i, rows, columns, positions) && decrease_score(
		positions, 0, i, rows, columns, currentPlayer)) {
            chosenPosition[0] = 0;
            chosenPosition[1] = i;
            return chosenPosition;
        }
    }
			
    for (i = 1; i < rows - 1; i++) {
        if (valid_position(i, columns - 1, rows, columns, positions)
  	        && decrease_score(positions, i, columns - 1, rows,
                columns, currentPlayer)) {
            chosenPosition[0] = i;
            chosenPosition[1] = columns - 1;
            return chosenPosition;
        }
    }

    for (i = columns - 2; i > 0; i--) {
        if (valid_position(rows - 1, i, rows, columns, positions) &&
                decrease_score(positions, rows - 1, i, rows, columns,
                currentPlayer)) {
            chosenPosition[0] = rows - 1;
            chosenPosition[1] = i;
            return chosenPosition;
        }
    }

    for (i = rows - 2; i > 0; i--) {
        if (valid_position(i, 0, rows, columns, positions) && decrease_score(
		positions, i, 0, rows, columns, currentPlayer)) {
            chosenPosition[0] = i;
            chosenPosition[1] = 0;
            return chosenPosition;
        }
    }
			
    int highScore = positions[0].score;
    for (i = 0; i < rows * columns; i++) {
        if (positions[i].score > highScore && valid_position(positions[i].row, 
		positions[i].column, rows, columns, positions)) {
            highScore = positions[i].score;
            chosenPosition[0] = positions[i].row;
            chosenPosition[1] = positions[i].column;
        }
    }
    return chosenPosition;
}

/* Carry out a move for player O when they are a human player
 * */
void human_o_move(char** board, int rows, int columns, Position* positions, 
	char* currentPlayer) {
    int chosenRow = 0, chosenColumn = 0;
    
    while (!valid_position(chosenRow, chosenColumn, rows, columns, 
	    positions)) {
        char* buffer = (char*)malloc(sizeof(char) * 81);
        printf("%c:(R C)> ", *currentPlayer);
        
        int c = fgetc(stdin), i = 1;
        if (c == EOF) {
            fprintf(stderr, "End of file\n");
            exit(5);
        }

        buffer[0] = c;
        while (c = fgetc(stdin), c != '\n' && c != EOF) {
            buffer[i] = c;
            i++;
        }
        buffer[i] = '\0';

        size_t len = strlen(buffer);

        if (buffer[0] == 's' && len > 1) {
            char* fileName = (char*)malloc(sizeof(char) * 80);
            int j;

            for (j = 1; j <= len; j++) {
                fileName[j - 1] = buffer[j];
            }
            save_game(board, rows, columns, fileName, currentPlayer);
        } else {
            sscanf(buffer, "%d %d", &chosenRow, &chosenColumn);
        }
    }

    int i;
    if (outer_position(chosenRow, chosenColumn, rows, columns)) {
        push_stones(chosenRow, chosenColumn, rows, columns, positions, 
		currentPlayer);
    } else {
        for (i = 0; i < rows * columns; i++) {
            if (positions[i].row == chosenRow && positions[i].column == 
		    chosenColumn) {
                positions[i].stone = *currentPlayer;
            }
        }   
    } 
    *currentPlayer = 'X';
}

/* Carry out a move for player X when they are a human player
 * */
void human_x_move(char** board, int rows, int columns, Position* positions, 
	char* currentPlayer) {
    int chosenRow = 0, chosenColumn = 0;

    while (!valid_position(chosenRow, chosenColumn, rows, columns, 
	    positions)) {
        char* buffer = (char*)malloc(sizeof(char) * 81);
        printf("%c:(R C)> ", *currentPlayer);

        int c = fgetc(stdin), i = 1;
        if (c == EOF) {
            fprintf(stderr, "End of file\n");
            exit(5);
        } 
        
        buffer[0] = c;
        while (c = fgetc(stdin), c != '\n' && c != EOF) {
            buffer[i] = c;
            i++;
        }
        buffer[i] = '\0';       

        size_t len = strlen(buffer);       

        if (buffer[0] == 's' && len > 1) {
            char* fileName = (char*)malloc(sizeof(char) * 80);

            int j;
            for (j = 1; j <= len; j++) {
                fileName[j - 1] = buffer[j];
            }
            save_game(board, rows, columns, fileName, currentPlayer);
        } else {
            sscanf(buffer, "%d %d", &chosenRow, &chosenColumn);
        }
        free(buffer);
    }

    int i;
    if (outer_position(chosenRow, chosenColumn, rows, columns)) {
        push_stones(chosenRow, chosenColumn, rows, columns, positions, 
		currentPlayer);
    } else {
        for (i = 0; i < rows * columns; i++) {
            if (positions[i].row == chosenRow && positions[i].column == 
		    chosenColumn) {
                positions[i].stone = *currentPlayer;
            }
        }
    }
    *currentPlayer = 'O';
}

/* Ensure that the given row and column gives a valid position
 * Return true if the position is on the interior and empty or on the edge
 * and produces a valid push
 * Return false otherwise
 * */
bool valid_position(int chosenRow, int chosenColumn, int rows, int columns, 
	Position* positions) {
    int i;
    Position chosenPosition;

    for (i = 0; i < rows * columns; i++) {
        if (positions[i].row == chosenRow && positions[i].column == 
		chosenColumn) {
            chosenPosition = positions[i];
        }
    }

    if (chosenRow < 0 || chosenRow > rows - 1) {
        return false;
    } else if (chosenRow == 0 && chosenColumn == 0) {
        return false;
    } else if (chosenRow == 0 && chosenColumn == columns - 1) {
        return false;
    } else if (chosenRow == rows - 1 && chosenColumn == 0) {
        return false;
    } else if (chosenRow == rows - 1 && chosenColumn == columns - 1) {
        return false;
    } else if (chosenPosition.stone != '.') {
        return false;
    } else if (outer_position(chosenRow, chosenColumn, rows, columns) && 
	    !valid_push(chosenRow, chosenColumn, rows, columns, positions)) {
        return false;
    } else {
        return true;
    }
}

/* Check if the given row and column give a position on the edge of the board
 * Return true if the position is on the edge and false otherwise
 * */
bool outer_position(int chosenRow, int chosenColumn, int rows, int columns) {
    if (chosenRow == 0 || chosenRow == rows - 1 || chosenColumn == 0 || 
	    chosenColumn == columns - 1) {
        return true;
    } else {
        return false;
    }
}

/* Ensure that playing a stone at the position given by the specified row and 
 * column would produce a valid push
 * A push is valid when there is an empty cell in the direction of the push
 * or there is a stone to be pushed immediately next to the position
 * Return true if the push is valid and false otherwise
 * */
bool valid_push(int chosenRow, int chosenColumn, int rows, int columns, 
	Position* positions) {
    int i;
    if (chosenRow == 0) {
        if (positions[chosenColumn + columns].stone == '.') {
            return false;
        } else {
            for (i = chosenColumn + (2 * columns); i < rows * columns; i += 
		    columns) {
                if (positions[i].stone == '.') {
                    return true;
                }
            }
            return false;
        }
    } else if (chosenRow == rows - 1) {
        if (positions[chosenRow * columns + chosenColumn - columns].stone == 
		'.') {
            return false;
        } else {
            for (i = columns * (chosenRow - 1) + chosenColumn; i >= 0; i -= 
		    columns) {
                if (positions[i].stone == '.') {
                    return true;
                }
            }
            return false;
        }
    } else if (chosenColumn == 0) {
        if (positions[chosenRow * columns + 1].stone == '.') {
            return false;
        } else {
            for (i = chosenColumn + 1; i < columns; i++) {
                if (positions[chosenRow * columns + i].stone == '.') {
                    return true;
                }
            }
            return false;
        }
    } else {
        if (positions[chosenRow * columns + chosenColumn - 1].stone == '.') {
            return false;
        } else {
            for (i = chosenColumn - 1; i >= 0; i--) {
                if (positions[chosenRow * columns + i].stone == '.') {
                    return true;
                }
            }
            return false;
        }
    }
}

/* If a push from the position given by the specified row and column is valid,
 * push the stones in the necessary direction
 * */
void push_stones(int chosenRow, int chosenColumn, int rows, int columns, 
	Position* positions, char* currentPlayer) {
    int chosenIndex = chosenRow * columns + chosenColumn;
    
    if (chosenColumn == 0) {
        right_push(positions, rows, columns, chosenIndex, currentPlayer);
    } else if (chosenColumn == columns - 1) {
        left_push(positions, rows, columns, chosenIndex, currentPlayer);
    } else if (chosenRow == 0) {
        downward_push(positions, rows, columns, chosenIndex, currentPlayer);
    } else {
        upward_push(positions, rows, columns, chosenIndex, currentPlayer);
    }
}

/* Push stones when a position in the first row is chosen
 * */
void downward_push(Position* positions, int rows, int columns, 
	int chosenIndex, char* currentPlayer) {
    int shiftIndex = chosenIndex, i;
    
    for (i = chosenIndex + columns; i <= chosenIndex + (columns *
            (rows - 1)); i += columns) {
        if (positions[i].stone == '.') {
            shiftIndex += columns;
            break;
        }
        shiftIndex += columns;
    }
    
    for (i = shiftIndex; i > chosenIndex + columns; i -= columns) {
        positions[i].stone = positions[i - columns].stone;
    }
    
    positions[chosenIndex + columns].stone = *currentPlayer;
}

/* Push stones when a position in the last column is chosen
 * */
void left_push(Position* positions, int rows, int columns, int chosenIndex, 
	char* currentPlayer) {
    int shiftIndex = chosenIndex, i;
    
    for (i = chosenIndex - 1; i > chosenIndex - columns; i--) {
        if (positions[i].stone == '.') {
            shiftIndex--;
            break;
        }
        shiftIndex--;
    }
    
    for (i = shiftIndex; i < chosenIndex; i++) {
        positions[i].stone = positions[i + 1].stone;
    }
    
    positions[chosenIndex - 1].stone = *currentPlayer;    
}

/* Push stones when a position in the last row is chosen
 **/
void upward_push(Position* positions, int rows, int columns, int chosenIndex, 
	char* currentPlayer) {
    int shiftIndex = chosenIndex, i;
    
    for (i = chosenIndex - columns; i >= chosenIndex - (columns *
            (rows - 1)); i -= columns) {
        if (positions[i].stone == '.') {
            shiftIndex -= columns;
            break;
        }
        shiftIndex -= columns;
    }
    
    for (i = shiftIndex; i < chosenIndex - columns; i += columns) {
        positions[i].stone = positions[i + columns].stone;
    }
    
    positions[chosenIndex - columns].stone = *currentPlayer;
}

/* Push stones when a position in the first column is chosen
 * */
void right_push(Position* positions, int rows, int columns, int chosenIndex, 
	char* currentPlayer) {
    int shiftIndex = chosenIndex, i;
    
    for (i = chosenIndex + 1; i < chosenIndex + columns; i++) {
        if (positions[i].stone == '.') {
            shiftIndex++;
            break;
        }
        shiftIndex++;
    }
    
    for (i = shiftIndex; i > chosenIndex + 1; i--) {
        positions[i].stone = positions[i - 1].stone;
    }
    
    positions[chosenIndex + 1].stone = *currentPlayer;
}

/* Read the board and calculate the score for player O
 * Return a pointer to the player's score
 * */
int* get_o_score(Position* positions, int rows, int columns) {
    int i;
    int* oScore = (int*)malloc(sizeof(int));
    *oScore = 0;
    
    for (i = 0; i < rows * columns; i++) {
        if (positions[i].stone == 'O') {
            *oScore += positions[i].score;
        }
    }

    return oScore;
}

/* Read the board and calculate the score for player X
 * Return a pointer to the player's score
 * */
int* get_x_score(Position* positions, int rows, int columns) {
    int i;
    int* xScore = (int*)malloc(sizeof(int));
    *xScore = 0;
    
    for (i = 0; i < rows * columns; i++) {
        if (positions[i].stone == 'X') {
            *xScore += positions[i].score;
        }
    }

    return xScore;
}

/* Check to see if a push from the position given by the specified row and
 * column would decrease the opposing player's score
 * Return true if the opposing player's score is lowered and false otherwise
 * */
bool decrease_score(Position* positions, int chosenRow, int chosenColumn, 
	int rows, int columns, char* currentPlayer) {
    int* otherScore = (int*)malloc(sizeof(int));

    if (*currentPlayer == 'O') {
        *otherScore = *get_x_score(positions, rows, columns);
    } else {
        *otherScore = *get_o_score(positions, rows, columns);
    }

    Position* temp = (Position*)malloc(sizeof(Position) * rows * columns);
    int i;
    for (i = 0; i < rows * columns; i++) {
        temp[i] = positions[i];
    }

    push_stones(chosenRow, chosenColumn, rows, columns, temp, currentPlayer);
    if (*currentPlayer == 'O') {
        if (*get_x_score(temp, rows, columns) < *otherScore) {
            return true;
        } else {
            return false;
        }
    } else {
        if (*get_o_score(temp, rows, columns) < *otherScore) {
            return true;
        } else {
            return false;
        }
    }
}

/* Check to see if the game is over
 * The game is over when all interior positions in the board are full
 * Return true if the game is over and false otherwise
 * */
bool game_over(Position* positions, int rows, int columns) {
    int i;
    
    for (i = 0; i < rows * columns; i++) {
        if (!outer_position(positions[i].row, positions[i].column, rows, 
		columns) && positions[i].stone == '.') {
            return false;
        }
    }

    return true;
}

/* Print the winning player after the game is over
 * Print both players if the game was a tie
 * */
void display_winners(Position* positions, int rows, int columns) {
    if (*get_o_score(positions, rows, columns) > *get_x_score(positions, rows, 
	    columns)) {
        printf("Winners: O\n");
    } else if (*get_x_score(positions, rows, columns) > 
	    *get_o_score(positions, rows, columns)) {
        printf("Winners: X\n");
    } else {
        printf("Winners: O X\n");
    }
}

/* Save the board to a file in a way that is readable
 * The dimensions of the board and the current player are printed to the 
 * top of the file
 * Exit if an error occurred when opening the output file
 * */
void save_game(char** board, int rows, int columns, char* fileName, 
	char* currentPlayer) {
    FILE* outputFile = fopen(fileName, "w");

    if (outputFile == 0) {
        fprintf(stderr, "Save failed\n");
    } else {
        fprintf(outputFile, "%d %d\n%c\n", rows, columns, currentPlayer[0]);
        int r, c;
        
        for (r = 0; r < rows; r++) {
            for (c = 0; c < columns * 2 + 1; c++) {
                fputc(board[r][c], outputFile);
            }
        }
        
        fclose(outputFile);
    }
}
