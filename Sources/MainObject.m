/* 
   InterPhone.m
	version	: o.2.o
	date	: 25.july 1992
	from	: jolly
*/

#import <appkit/appkit.h>
#import "MainObject.h"
#import "MainObject.c"



@implementation MainObject

-appDidInit:sender
{
	unsigned int wnd;
	id speaker = [NXApp appSpeaker];
	
	cthread_set_limit(MAXTHREADS);
	
	
	init_mutex();
	
	connect=0;
	ssok=-1;
	csok=-1;
	realsok=-1;
	
	cthread_detach(cthread_fork((cthread_fn_t)startserver,(any_t)self));
	
        NXConvertWinNumToGlobal([[NXApp appIcon] windowNum], &wnd);
	[speaker setSendPort: NXPortFromName(NX_WORKSPACEREQUEST, NULL)];
        [speaker registerWindow:wnd toPort:[[NXApp appListener] listenPort] ];
	
	return self;
}

-appWillTerminate:sender
{
	if(ssok) shutdown(ssok,2);
	if(csok) shutdown(csok,2);
	shutdown(realsok,2);
	close(realsok);
	if(subpid) kill(subpid,15);
	subpid=0;
	return (void *)&realsok; // non - nil
}




- (int)iconEntered:(int)windowNum at:(double)x :(double)y
	iconWindow:(int)iconWindowNum 
	iconX:(double)iconX iconY:(double)iconY
	iconWidth:(double)iconWidth 
	iconHeight:(double)iconHeight
    	pathList:(char *)pathList
{
	if (!iconPathList || strcmp(iconPathList, pathList))
	{
		NX_FREE(iconPathList);
		NX_MALLOC(iconPathList, char, strlen(pathList)+1);
		strcpy(iconPathList, pathList);
	}
	return 0;
}



- (int)iconReleasedAt:(double)x :(double)y ok:(int *)flag
{
	if(strlen(iconPathList)>3)
		if(!strcmp( (iconPathList+strlen(iconPathList)-4),".snd"))
			*flag = YES;
	return 0;
}



- dwWindow:sender
{
	if(connect) return self;
	[DW makeKeyAndOrderFront:self];
	return self;
}



- cwButton:sender
{
	if(connect==2 || connect==0)
	{
		if(ssok!=-1)
		{
			shutdown(ssok,2);
			close(ssok);
		}
		if(csok!=-1)
		{
			shutdown(csok,2);
			close(csok);
		}
		connect=0;
		ssok=-1;
		csok=-1;

		if(subpid)
		{
			kill(subpid,15);
			subpid=0;
		}
		[CWStatus setStringValue:"No Connection(r1)"];
		[CW display];
		return self;
	}
	
	
	if(ssok==-1 && csok!=-1)
	{
		[CWStatus setStringValue:"Connecting\n(r2)"];
		[CW display];
		return self;
	}


	if(csok==-1 && ssok!=-1)
	{	
		connect2(self);
		cthread_detach(cthread_fork((cthread_fn_t)recorder,(any_t)self));	
 	   	subpid=player(self);
		[CWStatus setStringValue:"Connected\n (r3)"];
		[CWButton setTitle:"Abort"];
		[CW display];
		connect=2;
		return self;
	}
	[CWStatus setStringValue:"Connect\n (r6)"];
	return self;
}



- dwButton:sender
{
	char mom[30];

	if(connect!=0 || csok!=-1 || ssok!=-1) return self;
	strcpy(mom,[DWAddr stringValue]);
	if(!str2ip(mom,&address,&port))return self;
	
	[CWButton setTitle:"Abort"];
	[CWname setStringValue:"Unknown"];
	[CWhost setStringValue:mom];
	[DW orderOut:self];	
	[CWStatus setStringValue:"Connecting\n(dw)"];	
	[CW makeKeyAndOrderFront:self];
	connect2(self);
	
    return self;
}


- (int)setrealsok:(int)a
{
	realsok=a;
	return 1;
}

- (int)getssok
{
	return ssok;
}
- (int)setssok:(int)a
{
	ssok=a;
	return 1;
}

- (int)getcsok
{
	return csok;
}
- (int)setcsok:(int)a
{
	csok=a;
	return 1;
}



- (int)getport
{
	return port;
}
- (int)getaddr
{
	return address;
}




- (int)getconnect
{
	return connect;
}

- (int)setconnect:(int)a
{
	connect=a;
	return 1;
}

- (int)connected2:(char *)a toUser:(char *)b onPort:(int)c
{
	char str[100];
		
	[CWname setStringValue:b];

	if([self getcsok]!=-1)
	{
		[CWStatus setStringValue:"Connection\n established.(r4)"];
		cthread_detach(cthread_fork((cthread_fn_t)recorder,(any_t)self));
		subpid=player(self);
		connect=2;
		[CW display];
		return 1;
	}
	
	port=c;
	address=inet_addr(a);
	sprintf(str,"%s %d",a,c);
	
	//[DW performClose:self];
	[DWAddr setEnabled:NO];
	[CWButton setTitle:"Accept"];
	[CWhost setStringValue:str];
	[CWStatus setStringValue:"Ringing\n(r5)"];
			
	[CW orderFront:self];
	[CW display];
	
	

	//[RingSnd performClick:self];
	return 1;
}

- (int)bye
{
	[CWButton setTitle:"Do nothing"];
	[CWStatus setStringValue:"Connection\n closed."];
	[CW display];
	shutdown(ssok,2);
	close(ssok);
	shutdown(csok,2);
	close(csok);
	csok=-1;
	ssok=-1;
	connect=0;
	if(subpid) kill(subpid,15);
	subpid=0;
	[DWAddr setEnabled:YES];

	return 1;
}

- (int)setstatus:(char *)a button:(char *)b;
{
	[CWStatus setStringValue:a];
	[CWButton setTitle:b];
	[CW display];
	return 1;
}

-(int)setsubpid:(int)a
{
	subpid=a;
	return 1;
}

@end
