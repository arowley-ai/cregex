#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "regex.h"


int main(int argc, char** argv)
{

	RegTest("(\\d)A(\\d)","99A13A213A");
	RegTest("(\\d)A((\\d+)\\d)","99A13A213A");

	return 0;
}



