#include <stdio.h>

struct Foo {

   static Foo* instance() {
      static Foo foo;
      return &foo;
   }

   Foo() {
      printf("Foo\n");
   }
   ~Foo() {
      printf("~Foo\n");
   }

   int data[0x100000000];
};

int main()
{
   printf("before Foo::instance()\n");
   Foo::instance();
   printf("after Foo::instance()\n");

   // Uncomment the following would lead stack overflow or segmentation fault (core dumped). 
   //Foo foo;
   return 0;
}