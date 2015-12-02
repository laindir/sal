#include "token.h"

void init(void);

void process_token(enum type type);

token top(void);

token rtop(void);

token rpop(void);

int alt;
