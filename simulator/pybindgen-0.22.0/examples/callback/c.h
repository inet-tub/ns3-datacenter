
typedef void (*Visitor) (int value, void *data);


void visit (Visitor visitor, void *data);

