#ifndef NUMERICTABLE_H__
#define NUMERICTABLE_H__

#include <vector>

class NumericTable {
	typedef std::vector<int> Numbers;

	int rows_;
	int cols_;
	int maxnumber_;
	Numbers numbers_;

public:
	NumericTable(int rows, int cols)
			: rows_(rows)
			, cols_(cols)
			, maxnumber_(rows * cols)
			, numbers_(rows * cols) {
		regenerate();
	}

	void regenerate();

	int getRows() const {
		return rows_;
	}

	int getCols() const {
		return cols_;
	}

	int getMaxNumber() const {
		return maxnumber_;
	}

	int getNumber(int row, int col) const {
		return numbers_[col + row*cols_];
	}

	int getNumber(int idx) const {
		return numbers_[idx];
	}
};

#endif //NUMERICTABLE_H__
