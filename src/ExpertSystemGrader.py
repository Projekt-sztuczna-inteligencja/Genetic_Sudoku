class ExpertSystemGrader:
    """
    Zaawansowany System Ekspercki z systemem wag punktowych.
    Implementuje 4 poziomy ludzkiej dedukcji.
    """

    def __init__(self, puzzle: list[int]):
        self.board = puzzle[:]
        self.domains = {i: set(range(1, 10)) for i in range(81) if self.board[i] == 0}
        self.update_all_domains()

        self.technique_counts = {
            "Naked Single": 0,
            "Hidden Single": 0,
            "Naked Pair": 0,
            "Pointing Line": 0
        }

    def update_domains_for_cell(self, index: int, value: int):
        row, col = index // 9, index % 9

        for i in range(9):
            r_idx, c_idx = row * 9 + i, i * 9 + col
            if r_idx in self.domains: self.domains[r_idx].discard(value)
            if c_idx in self.domains: self.domains[c_idx].discard(value)

        start_row, start_col = (row // 3) * 3, (col // 3) * 3
        for r in range(start_row, start_row + 3):
            for c in range(start_col, start_col + 3):
                sq_idx = r * 9 + c
                if sq_idx in self.domains: self.domains[sq_idx].discard(value)

    def update_all_domains(self):
        for i in range(81):
            if self.board[i] != 0:
                self.update_domains_for_cell(i, self.board[i])

    # --- TECHNIKA 1: BARDZO ŁATWA ---
    def rule_naked_single(self) -> bool:
        to_remove = []
        for idx, domain in self.domains.items():
            if len(domain) == 1:
                val = domain.pop()
                self.board[idx] = val
                to_remove.append((idx, val))

        if to_remove:
            for idx, val in to_remove:
                del self.domains[idx]
                self.update_domains_for_cell(idx, val)
            self.technique_counts["Naked Single"] += len(to_remove)
            return True
        return False

    # --- TECHNIKA 2: ŁATWA ---
    def rule_hidden_single(self) -> bool:
        for val in range(1, 10):
            for sq in range(9):
                possible_spots = []
                start_row, start_col = (sq // 3) * 3, (sq % 3) * 3
                for r in range(start_row, start_row + 3):
                    for c in range(start_col, start_col + 3):
                        idx = r * 9 + c
                        if idx in self.domains and val in self.domains[idx]:
                            possible_spots.append(idx)

                if len(possible_spots) == 1:
                    idx = possible_spots[0]
                    self.board[idx] = val
                    del self.domains[idx]
                    self.update_domains_for_cell(idx, val)
                    self.technique_counts["Hidden Single"] += 1
                    return True
        return False

    # --- TECHNIKA 3: ŚREDNIA ---
    def rule_naked_pair(self) -> bool:
        made_progress = False

        for row in range(9):
            row_cells = [row * 9 + c for c in range(9) if row * 9 + c in self.domains]

            pairs = []
            for c1 in range(len(row_cells)):
                for c2 in range(c1 + 1, len(row_cells)):
                    idx1, idx2 = row_cells[c1], row_cells[c2]
                    dom1, dom2 = self.domains[idx1], self.domains[idx2]
                    if len(dom1) == 2 and dom1 == dom2:
                        pairs.append((dom1, idx1, idx2))


            for pair_vals, idx1, idx2 in pairs:
                for idx in row_cells:
                    if idx != idx1 and idx != idx2:
                        for val in pair_vals:
                            if val in self.domains[idx]:
                                self.domains[idx].discard(val)
                                made_progress = True

        if made_progress:
            self.technique_counts["Naked Pair"] += 1
        return made_progress

    # --- TECHNIKA 4: TRUDNA ---
    def rule_pointing_line(self) -> bool:
        made_progress = False
        for sq in range(9):
            start_row, start_col = (sq // 3) * 3, (sq % 3) * 3
            sq_cells = [r * 9 + c for r in range(start_row, start_row + 3) for c in range(start_col, start_col + 3) if
                        r * 9 + c in self.domains]

            for val in range(1, 10):
                spots_with_val = [idx for idx in sq_cells if val in self.domains[idx]]
                if not spots_with_val or len(spots_with_val) < 2: continue

                rows = set(idx // 9 for idx in spots_with_val)
                if len(rows) == 1:
                    row = rows.pop()
                    for c in range(9):
                        idx = row * 9 + c
                        if idx in self.domains and idx not in spots_with_val and val in self.domains[idx]:
                            self.domains[idx].discard(val)
                            made_progress = True

                cols = set(idx % 9 for idx in spots_with_val)
                if len(cols) == 1:
                    col = cols.pop()
                    for r in range(9):
                        idx = r * 9 + col
                        if idx in self.domains and idx not in spots_with_val and val in self.domains[idx]:
                            self.domains[idx].discard(val)
                            made_progress = True

        if made_progress:
            self.technique_counts["Pointing Line"] += 1
        return made_progress

    def grade_puzzle(self) -> str:
        """Silnik Wnioskujący z systemem wagowym."""
        while self.domains:
            if self.rule_naked_single(): continue
            if self.rule_hidden_single(): continue
            if self.rule_naked_pair(): continue
            if self.rule_pointing_line(): continue
            break

        # SYSTEM PUNKTOWY
        score = (
                self.technique_counts["Naked Single"] * 1 +
                self.technique_counts["Hidden Single"] * 5 +
                self.technique_counts["Naked Pair"] * 25 +
                self.technique_counts["Pointing Line"] * 40
        )

        if len(self.domains) > 0:
            return f"EVIL (Wymaga zgadywania / Score: {score}+)"
        elif self.technique_counts["Pointing Line"] > 0 or self.technique_counts["Naked Pair"] > 2:
            return f"HARD (Score: {score})"
        elif self.technique_counts["Hidden Single"] > 5 or self.technique_counts["Naked Pair"] > 0:
            return f"MEDIUM (Score: {score})"
        else:
            return f"EASY (Score: {score})"