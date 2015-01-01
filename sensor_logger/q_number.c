/*
 * q_number.c
 *
 *  Created on: Jan 1, 2015
 *      Author: Mitchel Pigsley
 */
#include <stdint.h>
#include "q_number.h"

int32_t ToQ(int32_t num)
{
	return num << Q_NUM;
}

int32_t QMultiply(int32_t a, int32_t b)
{

	int64_t mult = ((int64_t)a) * ((int64_t)b);
	return (int32_t)(mult >> Q_NUM);
}

int32_t QDivide(int32_t a, int32_t b)
{
	int64_t preMult = ((int64_t)a) << Q_NUM;
	return (int32_t)(preMult / b);
}
