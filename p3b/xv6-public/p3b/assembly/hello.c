#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

void hello(int arg) {
	printf(1, "Hello %d\n", arg);
	exit();
}

int main(void) {
	hello(1);
	exit();
}
