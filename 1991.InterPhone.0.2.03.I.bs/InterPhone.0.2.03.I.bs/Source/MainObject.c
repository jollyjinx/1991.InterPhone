/* 
   InterPhone.c
	version	: o.3.o
	date	: aug 1992
	from	: jolly
*/

#import <sys/socket.h>
#import <netinet/in.h>
#import <arpa/inet.h>
#import <netdb.h>



#import "MainObject.soc"

char * reads(int);






int str2ip(char *mom, int *adr,int *port)
{
	struct in_addr iadr;
	int i;
	u_short p=0;	
	
	i=strlen(mom);
	while(i>=0 && !p)
	{
		if(mom[i--]==' ')
		{
			p=atoi(mom+2+i);
			mom[i+1]=0;
		}
	}
	
		
	iadr.s_addr=inet_addr(mom);		
	if(iadr.s_addr==-1)
	{			
		char m[25];
		struct hostent *in2;
		
		in2=gethostbyname(mom);
		if(in2==NULL) return 0;

		sprintf(m,"%d",(u_char)*(in2->h_addr));		
		for(i=1;i<(int)(in2->h_length);i++)
			sprintf(m,"%s.%d",m,(u_char)*(in2->h_addr+i));
		iadr.s_addr=inet_addr(m);
	}

	strcpy(mom,inet_ntoa(iadr));
	
	*adr=iadr.s_addr;
	if(!p) *port=0x2001;
	else	*port=p;
	sprintf(mom,"%s   %d",mom,*port);
	return 1;
}




void startserver(id obj)
{
	static int 	s=-1;
	static struct sockaddr_in sadr;
	int len=sizeof(sadr);
	int moms;
	char str[100],str2[100],mom[100];
	char  * sooptions;
	int connect;
	int lsok;
	int port;
	
	
	lsok=s;
	connect=[obj getconnect];
	

	if(s==-1)
{
	s=0;	
	bzero(&sadr,sizeof(sadr));
	sadr.sin_family=AF_INET;
	sadr.sin_port=htons((u_short)PORT);
	sadr.sin_addr.s_addr=INADDR_ANY;
	
	s=socket(PF_INET,SOCK_STREAM,0);

	if(s==-1)
	{
		fprintf(stderr,"Create Socket Error \n");
		exit(1);
	}
	
	while(bind(s,(struct sockaddr *)&sadr,sizeof(sadr))==-1)
	{
		fprintf(stderr,"Socket bind Error. Please wait ( up to one minute )\n");
		sleep(5);
	} 
	if(setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&sooptions,sizeof(sooptions))==-1)
	{
		fprintf(stderr,"Set Sockopt Error\n");
		exit(1);
	}
	if(listen(s,1)==-1)
	{
		fprintf(stderr,"Socket listen Error\n");
		exit(1);
	}
	[obj setrealsok:s];

#ifdef VERBOSE
mutex_lock(prt);
fprintf(stderr,"socket bound B)\n");
mutex_unlock(prt);
#endif
}
	moms=accept(s,(struct sockaddr *)&sadr,&len);	
	if(moms==-1)
	{
		fprintf(stderr,"Socket accept error\n");
		exit(1);
	}
	cthread_detach(cthread_fork((cthread_fn_t)startserver, (any_t)obj));

	WRITES(moms,"NeXT_Sound_Server_by_Jolly\n");
	strcpy(str,reads(moms));
	if(strcmp(str,"NeXT_Sound_Client_by_Jolly"))
	{
		WRITES(moms,"Sorry.\n");
		fprintf(stderr,"Sorry\n");
		[obj setssok:-1];
		shutdown(moms,2);
		cthread_exit(0);
	}
	if([obj getconnect]==2) 
	{
		WRITES(moms,"Busy.\n");
		fprintf(stderr,"Busy.\n");
		shutdown(moms,2);
		cthread_exit(0);
	}

	[obj setconnect:1];	
	[obj setssok:moms];
	WRITES(moms,"connection established\n");
	{
		struct sockaddr name;
		int namelen;
		
		namelen=sizeof(struct sockaddr);
		getpeername(moms,&name,&namelen);
		
		sprintf(str,"%d.%d.%d.%d",(u_char)*((char*)&name+4),
			(u_char)*((char*)&name+5),(u_char)*((char*)&name+6),
			(u_char)*((char*)&name+7));
	}
	strcpy(str2,reads(moms));
	strcpy(mom,reads(moms));
	sscanf(mom,"%d",&port);
	[obj connected2:str toUser:str2 onPort:port];
	cthread_exit(0);
}










int connect2(id obj)
{
	struct 	sockaddr_in sadr;
	char 	ans[100];
	int cs=0;

#ifdef VERBOSE
mutex_lock(prt);
fprintf(stderr,"connect2 is called\n");
mutex_unlock(prt);
#endif

	
	sadr.sin_family=AF_INET;
	sadr.sin_port=htons([obj getport]);
	sadr.sin_addr.s_addr=[obj getaddr];

	cs=socket(PF_INET,SOCK_STREAM,0);
	if(cs==-1)
	{
		fprintf(stderr,"csCan't create socket.\n");
		exit(1);
	}
	if(connect(cs,(struct sockaddr *)&sadr,sizeof(sadr))==-1)
	{
		fprintf(stderr,"csCan't open connection. errno:%d\n",errno);
		[obj setstatus:"CanÁt open\nconnection" button:"Do nothing"];
		[obj setconnect:0];
		[obj setcsok:-1];
		close(cs);
		return 0;
	}
	
	strcpy(ans,reads(cs));
	if(strcmp(ans,"NeXT_Sound_Server_by_Jolly"))
	{
		[obj setstatus:"Not a Sound\nServer" button:"Do nothing"];
		[obj setcsok:-1];
		[obj setconnect:0];
		return 0;
	}
	[obj setcsok:cs];

#ifdef VERBOSE
mutex_lock(prt);	
fprintf(stderr,"server there\n");
mutex_unlock(prt);
#endif

	WRITES(cs,"NeXT_Sound_Client_by_Jolly\n");
	strcpy(ans,reads(cs));
	if(strcmp(ans,"connection established"))
	{
		[obj setstatus:"Busy" button:"Do nothing"];
		[obj setcsok:-1];
		[obj setconnect:0];
		return 0;
	}
	if(getlogin()==NULL) strcpy(ans,"Anonymous");
	else strcpy(ans,getlogin());


	WRITES(cs,ans);
	WRITES(cs,"\n");
	WRITES(cs,"8193\n");
	
	return 1;
}







char * reads(s)
int s;
{
	char ans[100];
	char buff[2];
	int a=0;

	buff[1]='\0';
	do
	{
		recv(s,buff,1,0);
		if(*buff==10) *buff='\0';
		if(*buff>=32 || *buff==0)
		ans[a++]=*buff;
	}
	while(*buff!='\0');
	return ans;
}

