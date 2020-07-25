#include <stdio.h>

/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for PA2, unidirectional or bidirectional
   data transfer protocols (from A to B. Bidirectional transfer of data
   is for extra credit and is not required).  Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

/* a "msg" is the data unit passed from layer 5 (teachers code) to layer  */
/* 4 (students' code).  It contains the data (characters) to be delivered */
/* to layer 5 via the students transport level protocol entities.         */
struct msg {
  char data[20];
  };

/* a packet is the data unit passed from layer 4 (students code) to layer */
/* 3 (teachers code).  Note the pre-defined packet structure, which all   */
/* students must follow. */
struct pkt {
   int seqnum;
   int acknum;
   int checksum;
   char payload[20];
    };

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/

// time ���� ����
float time = 0.000; // �Ʒ� �ִ� time ���� ���� ������
struct pkt_Timer { // Ÿ�̸�
    int seq; // �ش� #
    int receivedACK; // ACKed�� 1, �ƴϸ� 0
    float limit_time; // �ش� #�� ��Ŷ�� limit time, �߻� time + RTT
};
struct pkt_Timer timer[1024];

// init���� �Է� ����
int window_size; 
float RTT;

// sender ���� ����
struct pkt Sender[1024]; // �۽���(���� �� �۽� ��Ŷ�� ������ ����)
int next_seq_num; // sender���� ������ ���� ��Ŷ�� ������ �ѹ�
int real_received_pktnum; // ������ layer5�κ��� ���� ��Ŷ�� ������ �ѹ�
int send_base;
int no_seq_buff_index; // ������ ���� �ʴ� ��Ŷ�� ���ۿ� �����ϱ� ���� index

// receiver ���� ����
struct pkt Receiver[1024]; // ������(���� �� ���� ��Ŷ�� ������ ����)
int rcv_base; // expected seq #

// A�� layer5�κ��� �޽����� ���� ����. B�� �����ؾ� ��
void A_output(struct msg message) {
    if (next_seq_num < send_base + window_size) { // seq #�� ������ ���� ��ȣ�̸�
        // ��Ŷ �۾�
        int i;
        for (i = 0; i < 20; i++) 
            Sender[real_received_pktnum].payload[i] = message.data[i]; // ���� �޽����� ��Ŷ�� ����
        Sender[real_received_pktnum].seqnum = real_received_pktnum;
        Sender[real_received_pktnum].acknum = 0; // ACK�� �ޱ� �������� 0�� ����
        Sender[real_received_pktnum].checksum = 0; // packet corrrupt�� �Ű� ���� �����Ƿ� ��� 0���� �ʱ�ȭ
        printf("A�� %d����!", real_received_pktnum);
        tolayer3(0, Sender[real_received_pktnum]); // �ش� ��Ŷ�� B���� �����ϱ� ���� layer3�� ��������

        // Ÿ�̸� �۾�
        timer[real_received_pktnum].seq = real_received_pktnum;
        timer[real_received_pktnum].limit_time = time + RTT; // �ش� ������ ��Ŷ�� ��� �ð�. ���� �ð� + RTT
        if (send_base == next_seq_num) { // ���̽� #�� ��Ŷ�� ���ؼ� Ÿ�̸� ����
            starttimer(0, RTT);
        }
        next_seq_num++; // next seq num�� real received pkt num�� ���� ����
        real_received_pktnum++;
    }
    else {
        // seq #�� ������ ���� ��ȣ�� �ƴϸ� GBNó�� ���ۿ� ����ǰų� ���� �������� �÷� ������.
        // ���⼭�� �����Ƿ� ���ۿ� �����ϵ��� �Ѵ�.
        // �� ��Ȳ���� real_received_pktnum�� next_seq_num���� Ŀ����.
        int i;
        // ���۸� ����
        for (i = 0; i < 20; i++)
            Sender[real_received_pktnum].payload[i] = message.data[i]; // ���� �޽����� ��Ŷ�� ����
        Sender[real_received_pktnum].seqnum = real_received_pktnum;
        Sender[real_received_pktnum].acknum = 0; // ACK�� �ޱ� �������� 0�� ����
        Sender[real_received_pktnum].checksum = 0;
        no_seq_buff_index++; // ���۸��� ���ڸ� count
        real_received_pktnum++; // next_seq_num�� �������� �ʰ� real_received_pktnum�� ����. next_seq_num ������ ��Ŷ���� ����(tolayer3)�� ���� ����!     
    }
    printf("send base, next seq : [%d,%d]\n", send_base, next_seq_num); // send base�� next seq #�� ���
    printf("buffered : %d", no_seq_buff_index); // ���۵� ��Ŷ�� ���� ���
}

/* need be completed only for extra credit */
void B_output(struct msg message) {

}

// layer3���κ��� ACK�� �޴� ���.
void A_input(struct pkt packet) { 
    if (Sender[packet.acknum].acknum == 1) { // �̹� ���� ACK���
        printf("%d �����Ѵ�.\n", packet.acknum);
        return;
    }
    printf("\nA�� %d��ũ �޾Ҵ�!\n", packet.acknum);
    Sender[packet.acknum].acknum = 1; // �޾Ҵٰ� 1�� ǥ��
    timer[packet.acknum].receivedACK = 1; // Ÿ�̸ӿ��� ǥ��

    int i;
    if (packet.acknum == send_base) { // ���� ACK�� send base�� ���� ACK���ٸ�
        i = send_base;
        while (Sender[i].acknum == 1) { // send_base�� ���� ���� seq #�� ���� ���� Ȯ������ ���� ���� ������ �ű�
            i++;
            send_base++;
        }
    }

    // ���� send_base���� nextseq ��������(���� ������ ����) ��� ack�� �ް� ������ send_base�� �������Ѽ�
    // ���� ���� ���ο� �������� ��, �� send_base�� next_seq_num�� ���� ��
    if (send_base == next_seq_num) {
        stoptimer(0); // ���� send_base�� ���� Ÿ�̸Ӵ� ����
    }

    // ���κ��� ���� ������ ���� ��Ŷ�� ���۸� �س��Ҵٸ�(no_seq_buff_index�� 0�� �ƴ϶��), �׸���
    // next_seq_num�� ������ ���� ���� ���� �ش�ȴٸ� �Ʒ� ���� �ݺ�
    while (no_seq_buff_index != 0 && next_seq_num < send_base + window_size) {

        // next_seq_num�� ������ ���� #�� ���� next_seq_num�� ���������Ƿ� ���� ���۸� �Ǿ� �ִ� ù ��ġ�� next_sesq_num���� ������ �� �ִ�.
        printf("A�� ���۸��� ��Ŷ ����!\n");
        tolayer3(0, Sender[next_seq_num]); // ���۸��� ���� ����
        timer[next_seq_num].seq = next_seq_num; // Ÿ�̸� ����
        timer[next_seq_num].receivedACK = 0;
        timer[next_seq_num].limit_time = time + RTT;
        if (send_base == next_seq_num) { // A_output����ó�� send_base�� ��쿡 timer ����
            starttimer(0, RTT);
        }
        next_seq_num++; // ������ŭ next seq #�� ������Ű��
        no_seq_buff_index--; // ���۸� �صξ��� ������ ���ҽ�Ŵ
    }
}

// A�� timer intterupt�� �ɸ� ���
void A_timerinterrupt() {
    int i;    
    int j = send_base;
    starttimer(0, RTT / 2); // ACK�� ������ ������� �ܼ��� timer�� ������ �׳� �ٽ� timer ���۵�

    for (i = 0; i < next_seq_num - send_base; i++) { // ������ ���� ���� ���´� ��Ŷ ���� ��ŭ �ݺ�

        // ���� ACK�� ���� ���ߴµ� ���� �ð�(time)�� timer�� ������ �Ѱ� �ð��� �Ѿ��ٸ� �ش� ��Ŷ�� loss�� ���̴�.
        if (Sender[j].acknum == 0 && time > timer[j].limit_time) {
            printf("Ÿ�̸� ���ͷ�Ʈ! %d �ٽú���!\n", Sender[j].seqnum); // Ÿ�̸� ���ͷ�Ʈ�� �߻��ϸ� �������ϰ� Ÿ�̸� �����
            stoptimer(0);
            starttimer(0, RTT); // Ÿ�̸� �����
            tolayer3(0, Sender[j]);
           
            timer[j].seq = j;
            timer[j].receivedACK = 0;
            timer[j].limit_time = time + RTT;
            break;
        }
    }
}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init(void) {
    int i;
    send_base = 0;
    next_seq_num = 0;
    next_seq_num = 0;
    send_base = 0;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

// B�� A�κ��� ���������� ��Ŷ�� ����
void B_input(struct pkt packet) {
    printf("B�� %d, %c ����\n", packet.seqnum, packet.payload[0]);
    
    int i;
    if (packet.seqnum == rcv_base) { // rcv_base�� �ޱ� ����ϴ� �������� ������ ��Ŷ�̴�.
        Receiver[rcv_base] = packet; 

        // ���� ������ ���� �ʰ� �����ؼ� ���۸� �ص� �������� ��Ŷ�� �����Ѵٸ� ���� ������
        while (Receiver[rcv_base].seqnum != -1) { // ���� ��Ŷ�� ���� ���� ���� seqnum�� -1�� �ʱ�ȭ�Ǿ� �ִ�.
            tolayer5(1, Receiver[rcv_base].payload);
            rcv_base++;
        }
        struct pkt ACK; // ������ ACK ��Ŷ ���� �� A���� ����
        ACK.acknum = packet.seqnum;
        ACK.seqnum = 0;
        ACK.checksum = 0;
        for (i = 0; i < 19; i++) // payload �κ��� �������� ����
            ACK.payload[i] = ' ';
        ACK.payload[19] = '\0';
        printf("%d ACK ����!", ACK.acknum);
        tolayer3(1, ACK);
    }
    else if (packet.seqnum > rcv_base && packet.seqnum < rcv_base + window_size) { // Out-Of-Order ������ window ���� ��Ŷ
        if (Receiver[packet.seqnum].seqnum != -1) {
            printf("�ߺ��� ��Ŷ������ ACK ����\n");
        }
        Receiver[packet.seqnum] = packet; // ���۸� �� ACK�� ������ ��. ��, tolayer5�� �÷��������� ����
        struct pkt ACK;
        ACK.acknum = packet.seqnum;
        ACK.seqnum = 0;
        ACK.checksum = 0;
        for (i = 0; i < 19; i++)
            ACK.payload[i] = ' ';
        ACK.payload[19] = '\0';
        printf("%d ACK ����!(Out-Of-Order!)", ACK.acknum);
        tolayer3(1, ACK);
    }
    else if (packet.seqnum >= rcv_base - window_size && packet.seqnum < rcv_base) { // �̹� �����ڰ� Ȯ�������� ���̶�, ACK�� �����ؾ� ��.
        if (Receiver[packet.seqnum].seqnum != -1) {
            printf("�ߺ��� ��Ŷ������ ACK ����\n");
        }
        struct pkt ACK; // ���� ACK ������ ������ tolayer5�� �÷��������� ����
        ACK.acknum = packet.seqnum;
        ACK.seqnum = 0;
        ACK.checksum = 0;
        for (i = 0; i < 19; i++)
            ACK.payload[i] = ' ';
        ACK.payload[19] = '\0';
        printf("%d ACK ����!", ACK.acknum);
        tolayer3(1, ACK);
    }
    else { // �� �̿��� ��Ŷ�� ����
        printf("�߸��� ��Ŷ ����\n");
    }
}

/* called when B's timer goes off */
void B_timerinterrupt(void) {

}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init(void) {
    rcv_base = 0;
    int i;
    for (i = 0; i < 1024; i++) {
        Receiver[i].seqnum = -1;
    }
}

/*****************************************************************
***************** NETWORK EMULATION CODE STARTS BELOW ***********
The code below emulates the layer 3 and below network environment:
  - emulates the tranmission and delivery (possibly with bit-level corruption
    and packet loss) of packets across the layer 3/4 interface
  - handles the starting/stopping of a timer, and generates timer
    interrupts (resulting in calling students timer handler).
  - generates message to be sent (passed from later 5 to 4)

THERE IS NOT REASON THAT ANY STUDENT SHOULD HAVE TO READ OR UNDERSTAND
THE CODE BELOW.  YOU SHOLD NOT TOUCH, OR REFERENCE (in your code) ANY
OF THE DATA STRUCTURES BELOW.  If you're interested in how I designed
the emulator, you're welcome to look at the code - but again, you should have
to, and you defeinitely should not have to modify
******************************************************************/

struct event {
   float evtime;           /* event time */
   int evtype;             /* event type code */
   int eventity;           /* entity where event occurs */
   struct pkt *pktptr;     /* ptr to packet (if any) assoc w/ this event */
   struct event *prev;
   struct event *next;
 };
struct event *evlist = NULL;   /* the event list */

/* possible events: */
#define  TIMER_INTERRUPT 0  
#define  FROM_LAYER5     1
#define  FROM_LAYER3     2

#define  OFF             0
#define  ON              1
#define   A    0
#define   B    1

int TRACE = 1;             /* for my debugging */
int nsim = 0;              /* number of messages from 5 to 4 so far */ 
int nsimmax = 0;           /* number of msgs to generate, then stop */
float lossprob;            /* probability that a packet is dropped  */
float corruptprob = 0;         /* ��Ŷ�� corrupt�� ������ ������� �����Ƿ� 0���� �ʱ�ȭ */
float lambda;              /* arrival rate of messages from layer 5 */   
int   ntolayer3;           /* number sent into layer 3 */
int   nlost;               /* number lost in media */
int ncorrupt;              /* number corrupted by media*/

main()
{
   struct event *eventptr;
   struct msg  msg2give;
   struct pkt  pkt2give;
   
   int i,j;
   char c; 
  
   init();
   A_init();
   B_init();
   
    while (1) {
        eventptr = evlist;            /* get next event to simulate */

        // ���� �̺�Ʈ ������
        if (eventptr==NULL)
           goto terminate;
        evlist = evlist->next;        /* remove this event from event list */
        if (evlist!=NULL)
           evlist->prev=NULL;

        // �̺�Ʈ ���� ���
        if (TRACE>=2) {
            printf("\nEVENT time: %f,",eventptr->evtime);
            printf("  type: %d",eventptr->evtype);
            if (eventptr->evtype==0)
	            printf(", timerinterrupt  ");
            else if (eventptr->evtype==1)
                printf(", fromlayer5 ");
            else
	            printf(", fromlayer3 ");
           printf(" entity: %d\n",eventptr->eventity);
           }
        time = eventptr->evtime;        /* update time to next event time */

        // ��������
        if (nsim==nsimmax)
	        break;                        /* all done with simulation */

        // ���ο� ��Ŷ ������
        if (eventptr->evtype == FROM_LAYER5 ) {
            generate_next_arrival();   /* set up future arrival */
            /* fill in msg to give with string of same letter */    
            j = nsim % 26; 
            for (i=0; i<20; i++)  
               msg2give.data[i] = 97 + j;
            if (TRACE>2) {
               printf("          MAINLOOP: data given to student: ");
                 for (i=0; i<20; i++) 
                  printf("%c", msg2give.data[i]);
               printf("\n");
	        }
            nsim++;
            if (eventptr->eventity == A) {
                A_output(msg2give);
            }
            else {
                B_output(msg2give);
            }
        }
        
        // ��ũ�� �޾��� ��
        else if (eventptr->evtype ==  FROM_LAYER3) {
            pkt2give.seqnum = eventptr->pktptr->seqnum;
            pkt2give.acknum = eventptr->pktptr->acknum;
            pkt2give.checksum = eventptr->pktptr->checksum;
            for (i=0; i<20; i++)  
                pkt2give.payload[i] = eventptr->pktptr->payload[i];
	        if (eventptr->eventity ==A)      /* deliver packet by calling */
   	            A_input(pkt2give);            /* appropriate entity */
            else
   	            B_input(pkt2give);
	        free(eventptr->pktptr);          /* free the memory for packet */
        }

        // Ÿ�̸� ���ͷ�Ʈ �߻���
        else if (eventptr->evtype ==  TIMER_INTERRUPT) {
            if (eventptr->eventity == A) 
	            A_timerinterrupt();
            else
	            B_timerinterrupt();
        }

        else {
	        printf("INTERNAL PANIC: unknown event type \n");
        }

        // ��º�
        printf("\n");
        for (i = 0; i < send_base; i++) // �̹� ack���� �� ���� ��Ŷ
            printf("$ ");
        for (i = send_base; i < next_seq_num; i++) // ack�� ���� ���߰�, ������ �� ��Ŷ
            printf("%d ", i);
        for (i = next_seq_num; i < send_base + window_size; i++) // window ���� ����� ǥ��
            printf(". ");
        printf("\n");
        for (i = 0; i < rcv_base; i++) { // rcv_base ������ ��Ŷ��
            if (Receiver[i].seqnum != -1) { 
                if (Sender[i].acknum == 1) // �޾Ұ�, sender�� ack���� ���� ���
                    printf("$ ");
                else {
                    printf("%d ", i); // �޾Ҵµ� sender�� ack�� ���� ���� ���
                }
            }
        }
        for (i = rcv_base; i < rcv_base + window_size; i++) { // ������ ���� �ʰ� ������ ��Ŷ
            if (Receiver[i].seqnum == -1)
                printf("  ");
            else { 
                if (Sender[i].acknum == 1) // sender���� ack���� �� ���� ���
                    printf("$ ");
                else
                    printf("%d ", i); // ���� sender�� ack�� ���� ���� ���
            }
        }
        printf("\n");

        free(eventptr);
    }

    terminate:
    printf(" Simulator terminated at time %f\n after sending %d msgs from layer5\n",time,nsim);
}



init()                         /* initialize the simulator */
{
  int i;
  float sum, avg;
  float jimsrand();
  
  
   printf("-----  Stop and Wait Network Simulator Version 1.1 -------- \n\n");
   printf("Enter the number of messages to simulate: ");
   scanf("%d",&nsimmax);
   printf("Enter  packet loss probability [enter 0.0 for no loss]:");
   scanf("%f",&lossprob);
   //printf("Enter packet corruption probability [0.0 for no corruption]:");
   //scanf("%f",&corruptprob); --> ��Ŷ�� corrupt�� ���� ������ �������� �־����� ����
   printf("Enter average time between messages from sender's layer5 [ > 0.0]:");
   scanf("%f",&lambda);
   printf("Enter the window size:");
   scanf("%d", &window_size);
   printf("Enter the timeout:");
   scanf("%f", &RTT);
   printf("Enter TRACE:");
   scanf("%d",&TRACE);

   srand(9999);              /* init random number generator */
   sum = 0.0;                /* test random number generator for students */
   for (i=0; i<1000; i++)
      sum=sum+jimsrand();    /* jimsrand() should be uniform in [0,1] */
   avg = sum/1000.0;
   if (avg < 0.25 || avg > 0.75) {
    printf("It is likely that random number generation on your machine\n" ); 
    printf("is different from what this emulator expects.  Please take\n");
    printf("a look at the routine jimsrand() in the emulator code. Sorry. \n");
    exit();
    }

   ntolayer3 = 0;
   nlost = 0;
   ncorrupt = 0;

   time=0.0;                    /* initialize time to 0.0 */
   generate_next_arrival();     /* initialize event list */
}

/****************************************************************************/
/* jimsrand(): return a float in range [0,1].  The routine below is used to */
/* isolate all random number generation in one location.  We assume that the*/
/* system-supplied rand() function return an int in therange [0,mmm]        */
/****************************************************************************/
float jimsrand() 
{
  double mmm = 32767;   /* largest int  - MACHINE DEPENDENT!!!!!!!!   */
  float x;                   /* individual students may need to change mmm */ 
  x = rand()/mmm;            /* x should be uniform in [0,1] */
  return(x);
}  

/********************* EVENT HANDLINE ROUTINES *******/
/*  The next set of routines handle the event list   */
/*****************************************************/
 
generate_next_arrival()
{
   double x,log(),ceil();
   struct event *evptr;
    char *malloc();
   float ttime;
   int tempint;

   if (TRACE>2)
       printf("          GENERATE NEXT ARRIVAL: creating new arrival\n");
 
   x = lambda*jimsrand()*2;  /* x is uniform on [0,2*lambda] */
                             /* having mean of lambda        */
   evptr = (struct event *)malloc(sizeof(struct event));
   evptr->evtime =  time + x;
   evptr->evtype =  FROM_LAYER5;
   evptr->eventity = A; // �޽��� sending�� ��ü�� �׻� A
   insertevent(evptr);
} 

insertevent(p)
   struct event *p;
{
   struct event *q,*qold;

   if (TRACE>2) {
      printf("            INSERTEVENT: time is %lf\n",time);
      printf("            INSERTEVENT: future time will be %lf\n",p->evtime); 
      }
   q = evlist;     /* q points to header of list in which p struct inserted */
   if (q==NULL) {   /* list is empty */
        evlist=p;
        p->next=NULL;
        p->prev=NULL;
        }
     else {
        for (qold = q; q !=NULL && p->evtime > q->evtime; q=q->next)
              qold=q; 
        if (q==NULL) {   /* end of list */
             qold->next = p;
             p->prev = qold;
             p->next = NULL;
             }
           else if (q==evlist) { /* front of list */
             p->next=evlist;
             p->prev=NULL;
             p->next->prev=p;
             evlist = p;
             }
           else {     /* middle of list */
             p->next=q;
             p->prev=q->prev;
             q->prev->next=p;
             q->prev=p;
             }
         }
}

printevlist()
{
  struct event *q;
  int i;
  printf("--------------\nEvent List Follows:\n");
  for(q = evlist; q!=NULL; q=q->next) {
    printf("Event time: %f, type: %d entity: %d\n",q->evtime,q->evtype,q->eventity);
    }
  printf("--------------\n");
}



/********************** Student-callable ROUTINES ***********************/

/* called by students routine to cancel a previously-started timer */
stoptimer(AorB)
int AorB;  /* A or B is trying to stop timer */
{
 struct event *q,*qold;

 if (TRACE>2)
    printf("          STOP TIMER: stopping timer at %f\n",time);
/* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
 for (q=evlist; q!=NULL ; q = q->next) 
    if ( (q->evtype==TIMER_INTERRUPT  && q->eventity==AorB) ) { 
       /* remove this event */
       if (q->next==NULL && q->prev==NULL)
             evlist=NULL;         /* remove first and only event on list */
          else if (q->next==NULL) /* end of list - there is one in front */
             q->prev->next = NULL;
          else if (q==evlist) { /* front of list - there must be event after */
             q->next->prev=NULL;
             evlist = q->next;
             }
           else {     /* middle of list */
             q->next->prev = q->prev;
             q->prev->next =  q->next;
             }
       free(q);
       return;
     }
  printf("Warning: unable to cancel your timer. It wasn't running.\n");
}

starttimer(AorB,increment)
int AorB;  /* A or B is trying to stop timer */
float increment;
{
 struct event *q;
 struct event *evptr;
 char *malloc();

 if (TRACE>2)
    printf("          START TIMER: starting timer at %f\n",time);
 /* be nice: check to see if timer is already started, if so, then  warn */
/* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
   for (q=evlist; q!=NULL ; q = q->next)  
    if ( (q->evtype==TIMER_INTERRUPT  && q->eventity==AorB) ) { 
      printf("Warning: attempt to start a timer that is already started\n");
      return;
      }
 
/* create future event for when timer goes off */
   evptr = (struct event *)malloc(sizeof(struct event));
   evptr->evtime =  time + increment;
   evptr->evtype =  TIMER_INTERRUPT;
   evptr->eventity = AorB;
   insertevent(evptr);
} 

/************************** TOLAYER3 ***************/
tolayer3(AorB,packet)
int AorB;  /* A or B is trying to stop timer */
struct pkt packet;
{
 struct pkt *mypktptr;
 struct event *evptr,*q;
 char *malloc();
 float lastime, x, jimsrand();
 int i;

 ntolayer3++;

 /* simulate losses: */
 if (jimsrand() < lossprob)  {
      nlost++;
      if (TRACE>0)    
	printf("          TOLAYER3: packet being lost\n");
      return;
    }  

/* make a copy of the packet student just gave me since he/she may decide */
/* to do something with the packet after we return back to him/her */ 
 mypktptr = (struct pkt *)malloc(sizeof(struct pkt));
 mypktptr->seqnum = packet.seqnum;
 mypktptr->acknum = packet.acknum;
 mypktptr->checksum = packet.checksum;
 for (i=0; i<20; i++)
    mypktptr->payload[i] = packet.payload[i];
 if (TRACE>2)  {
   printf("          TOLAYER3: seq: %d, ack %d, check: %d ", mypktptr->seqnum,
	  mypktptr->acknum,  mypktptr->checksum);
    for (i=0; i<20; i++)
        printf("%c",mypktptr->payload[i]);
    printf("\n");
   }

/* create future event for arrival of packet at the other side */
  evptr = (struct event *)malloc(sizeof(struct event));
  evptr->evtype =  FROM_LAYER3;   /* packet will pop out from layer3 */
  evptr->eventity = (AorB + 1) % 2; /* event occurs at other entity */
  evptr->pktptr = mypktptr;       /* save ptr to my copy of packet */
/* finally, compute the arrival time of packet at the other end.
   medium can not reorder, so make sure packet arrives between 1 and 10
   time units after the latest arrival time of packets
   currently in the medium on their way to the destination */
 lastime = time;
/* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next) */
 for (q=evlist; q!=NULL ; q = q->next) 
    if ( (q->evtype==FROM_LAYER3  && q->eventity==evptr->eventity) ) 
      lastime = q->evtime;
 evptr->evtime =  lastime + 1 + 9*jimsrand();


 /* simulate corruption: */
 if (jimsrand() < corruptprob)  {
    ncorrupt++;
    if ( (x = jimsrand()) < .75)
       mypktptr->payload[0]='Z';   /* corrupt payload */
      else if (x < .875)
       mypktptr->seqnum = 999999;
      else
       mypktptr->acknum = 999999;
    if (TRACE>0)    
	printf("          TOLAYER3: packet being corrupted\n");
    }  

  if (TRACE>2)  
     printf("          TOLAYER3: scheduling arrival on other side\n");
  insertevent(evptr);
} 

tolayer5(AorB,datasent)
  int AorB;
  char datasent[20];
{
  int i;  
  if (TRACE>2) {
     printf("          TOLAYER5: data received: ");
     for (i=0; i<20; i++)  
        printf("%c",datasent[i]);
     printf("\n");
   }
  
}