/*
 * q_number.h
 *
 *  Created on: Jan 1, 2015
 *      Author: Mitchel Pigsley
 */

#ifndef Q_NUMBER_H_
#define Q_NUMBER_H_

#define Q_NUM 18
#define Round(x) ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))

int32_t ToQ(int32_t num);
int32_t QMultiply(int32_t a, int32_t b);
int32_t QDivide(int32_t a, int32_t b);

#endif /* Q_NUMBER_H_ */
