#define requires_two(a,b) 0
#define variadic_test(...) requires_two(__VA_ARGS__)
int main(){return variadic_test(a,b);}
