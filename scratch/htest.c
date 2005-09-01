#include <stdio.h>
#include "htable.h"

int main()
{
    htable_ptr h = htable_new();
    int i;

    htable_put(h, (void *) 456, (void *) 123);
    htable_put(h, (void *) 0, (void *) 789);

    for (i=120; i<130; i++) {
	printf("%d: ", i);
	if (htable_has(h, (void *) i)) {
	    printf("%d\n", (int) htable_at(h, (void *) i));
	} else {
	    printf("nothing\n");
	}
    }
    return 0;
}
