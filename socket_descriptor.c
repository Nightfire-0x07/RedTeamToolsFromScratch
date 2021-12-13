/*
 *socket descriptor is represented by a standard file descriptor.
 *Unix: all file descriptors are small non negative descriptors
 * Windows: socket handle can be anything. 
 * Unix: socket() returns an int, and Windows: socket() returns a SOCKET
 * SOCKET is a typedef for an unsigned int in Winsock headers 
 * workaround below (store socket descriptor as SOCKET type on all platforms
 */

#if !defined(_WIN32)
#define socket INT
#endif

/* windows, socket() returns INVALID_SOCKET upon failure. Unix, socket()
 * returns a negative number for failure. workaround for all platforms (
 * macro to determine if socket descriptor is valid or not):
 */

#if defined(_WIN32)
#define ISVALIDSOCKET(s) ((s) != INVALID_SOCKET)
#else 
#define ISVALIDSOCKET(s) ((s) >= 0)
#endif

/* Closing sockets
 * all sockets on unix systems are also standard file descriptors i.e. sockets
 * on unix systems can  be closed using the standard close() function. 
 * Windows: special close function used instead: closesocket(). Macro:
 */

#if defined(_WIN32)
#define CLOSESOCKET(s) closesocket(s)
#else 
#define CLOSESOCKET(s) close(s)
#endif

/* error handling: Unix: error numbers get stored in thread-global errno variabled
 * (for functions such as socket(), bind(), accept() and etc.
 * Windows: error number retrieved by calling WSAGetLastError()
 * Macro:
 */

#if defined(_WIN32)
#define GETSOCKETERRNO() (WSAGetLastError())
#else
#define GETSOCKETERRNO() (errno)
#endif
