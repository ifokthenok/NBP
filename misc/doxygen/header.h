/**
 * @file header.h
 * @brief Brife disciption for this file.
 * 
 * Here is the detail discription for this file.
 * Run doxywizard in your shell, this cmd will lead you to complete the generation of the doxygen doc.
 * USE_MATHJAX = YES
 * 
 */


/** @file structcmd.h
@brief A Documented file.
Details.
*/

/** @def MAX(a,b)
@brief A macro that returns the maximum of \a a and \a b.
Details for MAX
@param a the first argument
@param b the second argument

*/

/** @var typedef unsigned int UINT32
@brief A type definition for a .
Details.
*/

/** @var int errno
@brief Contains the last error code.
@warning Not thread safe!
*/

/** @fn int open(const char *pathname,int flags)
@brief Opens a file descriptor.
@param pathname The name of the descriptor.
@param flags Opening flags.
*/

/** @fn int close(int fd)
@brief Closes the file descriptor \a fd.
@param fd The descriptor to close.
*/

/** @fn size_t write(int fd,const char *buf, size_t count)
@brief Writes \a count bytes from \a buf to the filedescriptor \a fd.
@param fd The descriptor to write to.
@param buf The data buffer to write.
@param count The number of bytes to write.
*/

/** @fn int read(int fd,char *buf,size_t count)
@brief Read bytes from a file descriptor.
@param fd The descriptor to read from.
@param buf The buffer to read into.
@param count The number of bytes to read.
*/

#define MAX(a,b) (((a)>(b))?(a):(b))
typedef unsigned int UINT32;
int errno;
int open( const char *, int );
int close( int );
size_t write( int , const char *, size_t );
int read( int , char *, size_t );


/**
 * @class MyClass
 * @brief Brife discription for MyClass. 
 *
 * Detail discription for MyClass.
 */
class MyClass {
public:
    int myVar1;     /**< My Var1 */

    
    float myVar2;   ///< My Var2


    /** My Var3 */
    double myVar3;



    /** @brief the definition of colour enum
    *
    * This enum define these colours which are used in system,
    * we can this enum as the colour identifier of system
    */
    enum TEnum {
        RED,        /**< enumidentify red */
        BLUE,       /**< enumidentify blue */
        YELLOW      /**< enumidentify yellow */
    } enumVar;

    
    /**
     * @brief Brife discription for MyFun1. 
     *
     * Detail disciption for MyFun1.
     * @param a The first operator
     * @param b THe second operator
     */
    int myFun1(int a, int b);

    /**
     * Brife discription for distance. 
     * Detail for distance. The distance between \f$(x_1,y_1)\f$ and \f$(x_2,y_2)\f$ is \f$\sqrt{(x_2-x_1)^2+(y_2-y_1)^2}\f$. 
     * @warning You need set JAVADOC_AUTOBRIEF = YES to make no \@brife needed.
     * @warning You need set  USE_MATHJAX = YES to generate this formulas
     * @param x The first operator
     * @param y THe second operator
     */
    int distance(int x, int y);

};


