// lab 6 21089537d
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUFFER_SIZE 256
#define NUMBER_OF_CHILD 4
#define CARDS 52
#define READY "R" // student can send this signal to parent to state that he finish on receive 
#define TOKEN "T" // parent will assign this token to the leading child

struct card{
   char suit;
   char rank;
   int played; // 1 means played, 0 means still in hand, works like tombstone
};

int getRank(struct card c){
   switch (c.rank)
   {
   case 'A': return 14;
   case 'K': return 13;
   case 'Q': return 12;
   case 'J': return 11;
   case 'T': return 10;
   case '9': return 9;
   case '8': return 8;
   case '7': return 7;
   case '6': return 6;
   case '5': return 5;
   case '4': return 4;
   case '3': return 3;
   case '2': return 2;
   default:
      return -1;
   }
}

int isLargerCard(struct card c1, struct card c2){
   // sort by suit first, than rank
   if (c1.suit == 'S') { // S is largest
      if (c2.suit != 'S') return 1; // c1 is larger
      if (getRank(c1) > getRank(c2)) return 1; // c1 is larger
      return 0; // c2 is larger
   }
   else if (c1.suit == 'H') {
      if (c2.suit == 'S') return 0; // c2 is larger
      if (c2.suit == 'H') {
         if(getRank(c1) > getRank(c2)) return 1; // c1 is larger
         else return 0;
      }
      return 1; // c2.suit = C/D
   }
   else if (c1.suit == 'C') {
      if (c2.suit == 'S' || c2.suit == 'H') return 0; // c2 is larger
      if (c2.suit == 'C') {
         if(getRank(c1) > getRank(c2)) return 1; // c1 is larger
         else return 0;
      }
      return 1; // c2.suit = D
   }
   else { // c1 is D
      if (c2.suit == 'D') if(getRank(c1) > getRank(c2)) return 1;
      return 0;
   }
}

void sortCards(struct card childCards[13]){
   int i, j;
   struct card c;
   for (i = 1; i < 13; i++){
      c = childCards[i];
      j = i - 1;
      while (j >= 0 && isLargerCard(c,childCards[j]) == 1) 
      {
         childCards[j+1] = childCards[j];
         j = j-1;
      }
      childCards[j+1] = c;
   }
}

struct card playSmallestCard(struct card childCards[13]){
   // reverse get by rank first, than suit
   int i, smallestPosition = 0; // set larger than 'A' (14)
   struct card smallestCard;
   smallestCard.rank = 'A';

   for (i = 0; i< 13; i++){
      if(childCards[i].played != 1){
         if(getRank(childCards[i]) <= getRank(smallestCard)) { // <= since it is sorted in S>H>C>D
            smallestPosition = i;
            smallestCard = childCards[i];
         }
      }
   }
   childCards[smallestPosition].played = 1;
   return smallestCard;
}

struct card playSmallerCard(struct card childCards[13], char s, char r){
   // reverse get by rank first, than suit
   int i, smallestPosition = 0;
   struct card smallestCard;
   smallestCard.suit = s;
   smallestCard.rank = r;

   for (i = 12; i>0; i--){
      if(childCards[i].played != 1 && childCards[i].suit == smallestCard.suit){ //look for same suit
         // if(getRank(childCards[i]) > getRank(smallestCard)) { // get a just larger card, seek from back to front
            childCards[i].played = 1;
            return childCards[i];
         // }
      }
   }
   if (smallestPosition == 0){
      // cannot find one with same suit, discard
      //If there is a chance of “discard” (i.e., no card with 
      // the same suit as the lead card) during playing, Q will be chosen for discard. If there is no Q, then the 
      // highest    card  will  be  discarded.  If  there  is  no    card,  the  highest  card  in  the  remaining  hand  will  be 
      // discarded. 
   }
   childCards[smallestPosition].played = 1;
   return smallestCard;
}

int main(int argc, char *argv[])
{
   int   childPids[NUMBER_OF_CHILD];
   int	p2cPipe[NUMBER_OF_CHILD][2];	// pipe sent from parent to child
   int	c2pPipe[NUMBER_OF_CHILD][2];	// pipe sent from child to parent
   char	buf[BUFFER_SIZE];
   int	i, n, cNum;

   for(cNum=0; cNum < NUMBER_OF_CHILD; cNum++) { // create pipe and child processes
      if (pipe(p2cPipe[cNum]) < 0) {
         printf("P2C Pipe creation error\n");
         exit(1);
      }
      if (pipe(c2pPipe[cNum]) < 0) {
         printf("C2P Pipe creation error\n");
         exit(1);
      }
      if ((childPids[cNum] = fork()) < 0) {
         printf("Fork failed\n");
         exit(1);
      } else if (childPids[cNum] == 0) { // child
         close(p2cPipe[cNum][1]); // close child out (This pipe is only read from parent)
         close(c2pPipe[cNum][0]); // close child in (This pipe is only write to parent)
         
         struct card childCards[13];
         int readyToplay = 0, receivedCards = 0, j = 0, isMyTurn = 0, ret;

         while (1){
            if ((n = read(p2cPipe[cNum][0],buf, BUFFER_SIZE)) > 0) { // read from pipe
               buf[n] = 0;
               // printf("<child %d> message [%s] of size %d bytes received\n", getpid(), buf, n);
               if (readyToplay == 0){ // get card stage
                  for (j = 0; j<n; j=j+2){ 
                     childCards[receivedCards].suit = buf[j];
                     childCards[receivedCards].rank = buf[j+1];
                     childCards[receivedCards].played = 0;
                     receivedCards++;
                  }
               }
               if (buf[0] == 'T') isMyTurn = 1;
            }
            if (receivedCards == 13 && readyToplay == 0){
               printf("Child %d pid %d: received", cNum+1, getpid());
               for (j = 0; j<13; j++) printf(" %c%c", childCards[j].suit, childCards[j].rank);

               printf("\nChild %d pid %d: arranged", cNum+1, getpid());
               sortCards(childCards); // sort his cards
               for (j = 0; j<13; j++) printf(" %c%c", childCards[j].suit, childCards[j].rank);
               printf("\n");

               write(c2pPipe[cNum][1], READY, sizeof(READY)); // send the ready-to play signal to parent
               readyToplay = 1;
            }
            if (readyToplay == 1 && isMyTurn == 1) {
               // printf("Child %d pid %d: now is my turn\n", cNum+1, getpid());
               if (n == 2){ // only receive the token, get the smallest card and sent to parent
                  struct card c = playSmallestCard(childCards);
                  char str[2];
                  str[0] = c.suit;
                  str[1] = c.rank;
                  printf("Child %d pid %d: play %c%c \n", cNum+1, getpid(), c.suit, c.rank);
                  ret = write(c2pPipe[cNum][1], str, 2); // play card
                  if (ret < 0) printf("write to pipe error!");
               }
               else{
                  //get just larger card than the previous player
                  char suit = buf[1];
                  char rank = buf[2];
                  struct card c = playSmallerCard(childCards, suit, rank);
                  char str[2];
                  str[0] = c.suit;
                  str[1] = c.rank;
                  printf("Child %d pid %d: play %c%c \n", cNum+1, getpid(), c.suit, c.rank);
                  ret = write(c2pPipe[cNum][1], str, 2); // play card
                  if (ret < 0) printf("write to pipe error!");
               }
               isMyTurn = 0;
            }
         }
         
         close(p2cPipe[cNum][0]);
         close(c2pPipe[cNum][1]);
         printf("<child %d> I have completed!\n", getpid());
         exit(0);
      } else { // parent
         close(p2cPipe[cNum][0]); // close parent in (This pipe is only read from child)
         close(c2pPipe[cNum][1]); // close parent out (This pipe is only receive from child)
      }
   }

   // wait outside fork()
   if(getpid() > 0){ // parent
   
      printf("Parent pid %d: child players are ", getpid());
      for (i = 0; i < NUMBER_OF_CHILD; i++) printf("%d ", childPids[i]);
      printf("\n");

      while ((n = read(STDIN_FILENO, buf, BUFFER_SIZE)) < 0); // wait until read a line

      char	cards[CARDS][2];
      char delim[] = " ";
      char *ptr = strtok(buf, delim);
      i = 0;
      while(ptr != NULL)
      {
         strcpy(cards[i], ptr);
         ptr = strtok(NULL, delim);
         i++;
      }
      for (i = 0; i < CARDS; i++){
         // printf("<parent> sending encrypted message [%s] to child %d\n",cards[i], i%NUMBER_OF_CHILD);
         if (i%NUMBER_OF_CHILD == 0) write(p2cPipe[0][1],cards[i], 2); // send the card to child 0
         else if (i%NUMBER_OF_CHILD == 1) write(p2cPipe[1][1],cards[i], 2); // send the card to child 1
         else if (i%NUMBER_OF_CHILD == 2) write(p2cPipe[2][1],cards[i], 2); // send the card to child 2
         else if (i%NUMBER_OF_CHILD == 3) write(p2cPipe[3][1],cards[i], 2); // send the card to child 3
      }

      int readyChilds = 0, round = 1, isPlaying = 0, j = 0, roundCount = 0, winningChild = 0;
      struct card largestCard, currentCard; // set as smallest at the begining
      char str[3];
      largestCard.suit = 'D';
      largestCard.rank = '1';

      struct card playedCards[NUMBER_OF_CHILD]; //to store each turn child's cards? needed?
      
      while (1) {
         // https://stackoverflow.com/questions/36242252/read-cards-from-multiple-pipes
         
         for(cNum = 0; cNum < NUMBER_OF_CHILD; cNum++) {
            if ((n = read(c2pPipe[cNum][0],buf, BUFFER_SIZE)) > 0) { // read from pipe
               buf[n] = 0;
               // printf("<parent> message from child %d [%s] of size %d bytes received\n",cNum+1,buf,n);
               if (strcmp(buf, READY) == 0) readyChilds++;
               if (isPlaying == 1){

                  roundCount++; 
                  // printf("Parent pid %d: roundCount %d, cNum %d\n", getpid(), roundCount, cNum);

                  printf("Parent pid %d: child %d plays %s\n", getpid(), cNum+1, buf);
                  str[0] = 'T'; // str[0] = TOKEN;
                  str[1] = buf[0]; currentCard.suit = buf[0];
                  str[2] = buf[1]; currentCard.rank = buf[1];

                  if (isLargerCard(currentCard, largestCard) == 1){
                     largestCard = currentCard;
                     winningChild = cNum + 1;
                     // printf("Parent pid %d: roundCount %d, winningChild %d, currentCard %c, largestCard %c \n", getpid(), roundCount, winningChild, currentCard.rank, largestCard.rank);
                  }

                  if (roundCount == 4){
                     roundCount = 0;
                     round++;
                     printf("Parent pid %d: child %d wins the trick\n", getpid(), winningChild);
                     printf("Parent pid %d: round %d child %d to lead\n", getpid(), round, winningChild);
                     cNum = winningChild - 1; // reset count
                     write(p2cPipe[cNum][1], TOKEN, sizeof(TOKEN)); // tell win child to start a new round
                     if (cNum < 3) cNum--;
                     largestCard.suit = 'D';
                     largestCard.rank = '1'; // reset to lowest card
                  }
                  else {
                     if (cNum == 3) cNum = -1; // reset the count
                     // printf("Parent pid %d: telling next child %d plays %s\n", getpid(), cNum+1, buf);
                     write(p2cPipe[cNum+1][1], str, sizeof(str)); // tell next child what card was played
                  }

               }
            }
         }
         if (readyChilds == 4){
            readyChilds = 0; isPlaying = 1; cNum = 0;
            printf("Parent pid %d: round %d child %d to lead\n", getpid(), round, cNum+1);
            write(p2cPipe[cNum][1], TOKEN, sizeof(TOKEN)); // pass TOKEN to that child
         }
         // sleep a little to avoid 100% CPU usage
         // sleep(1);
      }

      printf("Parent, pid %d: am i done?\n", getpid());
      for(i = 0; i < NUMBER_OF_CHILD; i++) {
         int retval;
         int cid = wait(&retval);
         if (WEXITSTATUS(retval) == 0) {
            for (j = 0; j < NUMBER_OF_CHILD; j++) {
               if (childPids[j] == cid) {
                  close(p2cPipe[i][1]);
                  close(c2pPipe[i][0]);
                  printf("Parent, pid %d: children %d completed execution\n", getpid(), j+1);
               }
            }
         }
      }
   }
   exit(0);
}
