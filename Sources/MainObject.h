/* InterPhone.h
	version	: o.2.o
	date	: 25.july 1992
	from	: jolly
*/

	
#import <objc/Object.h>
#import <appkit/nextstd.h>
#import <signal.h>

#ifndef PORT
#define PORT 0x2001
#endif

#define SNDLEN 4000		// small enough to prevent big time-lags eh ?

#define BUFFNO 4

#define MAXTHREADS 50
#define	WRITES(a,b) send(a,b,strlen(b),0)
#define SQUELCH 50
#undef VERBOSE


@interface MainObject:Object
{

    id	CW;
    id	CWhost;
    id	CWname;
    id	CWButton;
    id	CWStatus;
    
    id  DW;
    id	DWAddr;
    id	DWButton;

    id	RingSnd;

	char *iconPathList;
	
	int port;
	int address;
	int connect;
	int realsok,ssok,csok;
	
	int subpid;
}

- (int)iconEntered:(int)windowNum at:(double)x :(double)y
    iconWindow:(int)iconWindowNum iconX:(double)iconX iconY:(double)iconY
    iconWidth:(double)iconWidth iconHeight:(double)iconHeight
    pathList:(char *)pathList;
- (int)iconReleasedAt:(double)x :(double)y ok:(int *)flag;

- dwWindow:sender;
- cwButton:sender;
- dwButton:sender;

- (int)setrealsok:(int)a;
- (int)getssok;
- (int)setssok:(int)a;
- (int)getcsok;
- (int)setcsok:(int)a;

- (int)getport;
- (int)getaddr;

- (int)getconnect;
- (int)setconnect:(int)a;
- (int)connected2:(char *)a toUser:(char *)b onPort:(int)c;
- (int)bye;
- (int)setstatus:(char *)a button:(char *)b;
- (int)setsubpid:(int)a;
@end
