extern "C" {
extern int asmfunc(int a); /* declare external asm function */
int gvar = 0; /* define global variable */
}
void main()
{
 int I = 5;
 I = asmfunc(I); /* call function normally */
 while(1);
}
