#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include "shared.h"

int load_word_dictionary(const char *filename);
int generate_secret_word(char *secret_word);
void check_guess(const char *secret, const char *guess, int *bulls, int *cows);
int validate_guess(const char *guess, const char *secret);
int is_word_guessed(int bulls, int word_length);

#endif // GAME_LOGIC_H

