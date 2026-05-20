#include <stdint.h>

__declspec(dllexport) void calculate_fitness(const int* population, float* scores, int num_boards) {
    for (int b = 0; b < num_boards; ++b) {

        // Wskaźnik na początek konkretnej planszy w jednowymiarowej tablicy
        const int* board = &population[b * 81];
        int total_score = 0;

        // Python dostarcza nam 9 idealnych kwadratów 3x3,
        // dlatego w C oceniamy tylko poprawność 9 wierszy i 9 kolumn.
        for (int i = 0; i < 9; ++i) {
            unsigned int row_mask = 0;
            unsigned int col_mask = 0;

            for (int j = 0; j < 9; ++j) {
                // Wzór dla wiersza 'i' oraz kolumny 'j' (z mapowania kwadratowego)
                int row_idx = (i / 3) * 27 + (j / 3) * 9 + (i % 3) * 3 + (j % 3);

                // Wzór dla kolumny 'i' oraz wiersza 'j'
                int col_idx = (j / 3) * 27 + (i / 3) * 9 + (j % 3) * 3 + (i % 3);

                int r_val = board[row_idx];
                int c_val = board[col_idx];

                // Zapisujemy unikalność cyfr używając masek bitowych
                if (r_val >= 1 && r_val <= 9) row_mask |= (1 << r_val);
                if (c_val >= 1 && c_val <= 9) col_mask |= (1 << c_val);
            }

            // Sprzętowe liczenie aktywnych bitów daje nam liczbę unikalnych znaków
            total_score += __builtin_popcount(row_mask);
            total_score += __builtin_popcount(col_mask);
        }

        // Max punktów: 9 wierszy * 9 + 9 kolumn * 9 = 162
        scores[b] = total_score / 162.0f;
    }
}