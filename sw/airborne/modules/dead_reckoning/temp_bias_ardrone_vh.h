// Temperature-bias LUT from measurement means
// Generated 25-Jul-2017

inline float bias_x(int temp) {
	// Lookup table
	static const float LUT[] = {
			0.068906,
			0.110847,
			0.097801,
			0.137829,
			0.113874,
			0.121209,
			0.108250,
			0.123496,
			0.117022,
			0.129154,
			0.079355,
			-0.092309,
			-0.130726,
			-0.156389,
			-0.158728,
			-0.192815,
			-0.184726,
			-0.189959,
			-0.170798,
			-0.154994,
			-0.162457,
			-0.160978,
			-0.152680,
			-0.158831,
			-0.151664,
			-0.125092,
			-0.127944,
			-0.083539,
			-0.030304,
			-0.015958,
			-0.031796,
			-0.033739,
			-0.046761,
			-0.033767,
			-0.027667,
			-0.007221,
			-0.007854,
			-0.009632,
			-0.003743,
			-0.010453,
			-0.016121,
			-0.001029,
			-0.016524,
			-0.009596,
			-0.017324,
			-0.032205,
			-0.012495,
			-0.045861,
			-0.020475,
			0.009767,
			-0.013174,
			-0.076494,
			-0.036913,
			-0.039260,
			-0.033932,
			-0.049222,
			-0.018140,
			-0.036504,
			-0.038031,
			0.050870,
			0.064185,
			0.099713,
			0.128782 };
	// Look up bias for temperature
	int i = temp - 117;
	if (i < 0) i = 0;
	if (i > 62) i = 62;
	return LUT[i];
}

inline float bias_y(int temp) {
	// Lookup table
	static const float LUT[] = {
			-0.281072,
			-0.274752,
			-0.283510,
			-0.257825,
			-0.259030,
			-0.236739,
			-0.253494,
			-0.293259,
			-0.273802,
			-0.231753,
			-0.224848,
			-0.157227,
			-0.162533,
			-0.203498,
			-0.151674,
			-0.121770,
			-0.097509,
			-0.102662,
			-0.088491,
			-0.036083,
			-0.080457,
			-0.038282,
			-0.019506,
			-0.042119,
			-0.038593,
			-0.017847,
			-0.035857,
			-0.080339,
			-0.016829,
			-0.024888,
			0.002139,
			-0.027438,
			-0.004519,
			0.039111,
			0.006436,
			0.070271,
			0.042251,
			0.030890,
			0.044590,
			0.033640,
			0.087414,
			0.121747,
			0.087558,
			0.096800,
			0.090561,
			0.124586,
			0.105823,
			0.093180,
			0.080501,
			0.105229,
			0.109988,
			0.200381,
			0.163422,
			0.213418,
			0.189111,
			0.169877,
			0.228249,
			0.216936,
			0.232007,
			0.214981,
			0.201886,
			0.105636,
			0.177496 };
	// Look up bias for temperature
	int i = temp - 117;
	if (i < 0) i = 0;
	if (i > 62) i = 62;
	return LUT[i];
}