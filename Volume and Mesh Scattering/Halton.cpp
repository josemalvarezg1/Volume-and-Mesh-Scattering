#include "Halton.h"

std::vector<int> calculate_base_b_expansion(int k, int b) {
	if (k > 0) {
		auto j_max = floor(log(k) / log(b));
		auto a = std::vector<int>(j_max + 1, 0);
		auto q = pow(b, j_max);
		for (auto j = 1; j <= j_max; j++) {
			a[j - 1] = floor(k / q);
			k = k - q * a[j - 1];
			q = q / b;
		}
		return a;
	}
	else
		return std::vector<int>{0};
}

void increment_base_b_expansion(std::vector<int>& a, int b) {
	auto carry = true;
	auto m = a.size();
	for (auto i = 0; i < m; i++) {
		if (carry) {
			if (a[i] == b - 1)
				a[i] = 0;
			else {
				a[i] += 1;
				carry = false;
			}
		}
	}
	if (carry)
		a.push_back(1);
}
bool isPrime(int nNum, std::vector<int> vnPrimes) {
	/* Tests if a number is evenly divisable by any prime numbers */
	// vnPrimes needs to be an order accending list of prime numbers, starting from 2
	int nLimit = static_cast<int>(sqrt(nNum));
	for (auto nPrime : vnPrimes) {
		// Only need to check numbers up to sqrt(nNum)
		if (nLimit < nPrime) break;
		if (nNum % nPrime == 0) return false;
	}
	return true;
}

int nth_prime(int nNth) {
	/* Returns the Nth prime number */
	if (nNth <= 0)
		return 0;
	std::vector<int> vnPrimes{ 2 };
	for (int i = 3; vnPrimes.size() < (unsigned int)nNth; i += 2) {
		if (isPrime(i, vnPrimes))
			vnPrimes.push_back(i);
	}
	// Return the last one found (the nth!)
	return vnPrimes.back();
}



std::vector<float> get_halton_sequence(int start, int end, int dimension) {
	auto b = nth_prime(dimension);
	auto q = 1.f / b;
	auto xn = 0.f;
	std::vector<float> xs;
	auto a = calculate_base_b_expansion(start, b);
	auto m = a.size();
	for (auto i = start; i <= end; i++) {
		xn = 0.f;
		q = 1.f / b;
		m = a.size();
		for (auto j = 0; j < m; j++) {
			xn += q * a[j];
			q /= b;
		}
		xs.push_back(xn);
		increment_base_b_expansion(a, b);
	}
	return xs;
}