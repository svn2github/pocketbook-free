#include <algorithm>
#include "NumericTable.hh"

void NumericTable::regenerate() {
	std::fill(numbers_.begin(), numbers_.end(), 0);
	for (int i = 0; i < getMaxNumber(); ++i) {
		while (1) {
			int idx = rand() % getMaxNumber();
			if (numbers_[idx] == 0) {
				numbers_[idx] = i + 1;
				break;
			}
		}
	}
}
