#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>

using namespace std;

/* portul folosit */
#define PORT 2908

/* codul de eroare returnat de anumite apeluri */
extern int errno;

typedef struct thData{
	int idThread; //id-ul thread-ului tinut in evidenta de acest program
	int cl1; string name_cl1;
  int cl2; string name_cl2;//descriptorul intors de accept
  int wins1 = 0, wins2 = 0; 
}thData;

static void *room(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
void save_name(void *arg, int nr_client, struct thData &tdL);
int game(int player1, struct thData &tdL);

char * itoa(int number);
char * strcatchar(int value);
pthread_t th[100];

int main ()
{
  struct sockaddr_in server;	// structura folosita de server
  struct sockaddr_in from;	
  int sd;		//descriptorul de socket 
  int pid;
  //pthread_t th[100];    //Identificatorii thread-urilor care se vor crea
	int i = 1;

  /* crearea unui socket */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("[server]Eroare la socket().\n");
      return errno;
    }

  /* utilizarea optiunii SO_REUSEADDR */
  int on = 1;
  setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
  
  /* pregatirea structurilor de date */
  bzero (&server, sizeof (server));
  bzero (&from, sizeof (from));
  
  /* umplem structura folosita de server */
  /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;	
  /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl (INADDR_ANY);
  /* utilizam un port utilizator */
    server.sin_port = htons (PORT);
  
  /* atasam socketul */
  if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
      perror ("[server]Eroare la bind().\n");
      return errno;
    }

  /* punem serverul sa asculte daca vin clienti sa se conecteze */
  if (listen (sd, 2) == -1)
    {
      perror ("[server]Eroare la listen().\n");
      return errno;
    }
  

  /* servim in mod concurent clientii...folosind thread-uri */
  while (1)
  {
      int client1, client2;
      thData * td; //parametru functia executata de thread     
      td=(struct thData*)malloc(sizeof(struct thData));
      unsigned int length = sizeof (from);

      printf ("[server]Asteptam la portul %d...\n",PORT);
      fflush (stdout);

      /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
      
      if ( (client1 = accept (sd, (struct sockaddr *) &from, &length)) < 0)
        {
          perror ("[server]Eroare la accept().\n");
          continue;
        }

      if ( (client2 = accept (sd, (struct sockaddr *) &from, &length)) < 0)
        {
          perror ("[server]Eroare la accept().\n");
          continue;
        }
	

      /* s-a realizat conexiunea, se astepta mesajul */
      	
      td->idThread=i++;
      td->cl1=client1;
      td->cl2=client2;
      printf ("[nouthread]\n ");
      pthread_create(&th[i], NULL, &room, td);	                
  }    

};

static void *room(void * arg)
{		
		struct thData tdL; 
		
    tdL= *((struct thData*)arg);	
		
    pthread_detach(pthread_self());		

    save_name((struct thData*)arg, 1, tdL);
    save_name((struct thData*)arg, 2, tdL);

    int player_starting = tdL.cl1;

    while(1)
    {
      int winner = game(player_starting, tdL);
      
      int loser;

      char bufw[100];
      char bufl[100];

      winner == tdL.cl1 ? loser = tdL.cl2 : loser = tdL.cl1;

      write(loser, "Would you like another game? [Y/N]", sizeof("Would you like another game? [Y/N]"));
      write(winner, "Would you like another game? [Y/N]", sizeof("Would you like another game? [Y/N]"));

      read (loser, bufl, sizeof(bufl));
      read (winner, bufw, sizeof(bufw));

      if(bufl[0] == 'N')
        {
          write(winner, "0", sizeof("0"));
          break;
        }

      if(bufw[0] == 'N')
        {
          write(loser, "0", sizeof("0"));
          break;
        }

      write(loser, "Let's go again", sizeof("Let's go again"));
      write(winner, "Let's go again", sizeof("Let's go again"));

      player_starting = loser;
    }

    /* am terminat cu acesti clienti, inchidem conexiunea */
    close ((intptr_t)arg);
		
    return(NULL);	
  		
};

int game(int player1, struct thData &tdL)
{
  int winner, player2, i, j, currentp, otherp, nrc=0, row, ok = 0, k, played_column;
  char buffer[100];
  char combo[100];

  player1 == tdL.cl1 ? player2 = tdL.cl2 : player2 = tdL.cl1;

  char matrix[100][100];
  
  matrix[0][0] = '#';

  for (j = 1; j <= 7; j++)
      matrix[0][j] = j + '0';
  for (i = 1; i <= 6; i++)
      matrix[i][0] = char(i+64);

  for (int i = 1; i <= 6; i++)
    for(int j = 1; j <= 7; j++)
      matrix[i][j] = '-';

  currentp = player1;
  otherp = player2;

  while(1)
    {
      
      //display matrix pl1 pl2
      for(i = 0; i <= 6; i++)
       {
        k = 0; 
        memset(buffer, 0, sizeof(buffer));
        for(j = 0; j <= 7; j++)
          {
          buffer[k++] = matrix[i][j];
          buffer[k++] = ' '; buffer[k++] = ' '; buffer[k++] = ' ';  
          }

        //printf("trimit catre client1 buffer = '%s'\n", buffer);
        write(otherp, buffer, sizeof(buffer));
        //printf("trimit catre client2 buffer = '%s'\n", buffer);
        write(currentp, buffer, sizeof(buffer));
       
       }



      if(nrc == 5) // someone connected 4
        {
        strcpy(buffer, "0");
        write(otherp, buffer, sizeof(buffer));
        write(currentp, buffer, sizeof(buffer));

        memset(buffer, 0, sizeof(buffer));
        strcpy(buffer, "Congratulations! You won with the combination: ");
        strcat(buffer, combo);
        strcat(buffer, "\n");
        write(otherp, buffer, sizeof(buffer));  
        
        memset(buffer, 0, sizeof(buffer));
        strcpy(buffer, "You lost! The other player won with the combination: ");
        strcat(buffer, combo);
        strcat(buffer, "\n");
        write(currentp, buffer, sizeof(buffer));

        winner = otherp;

        winner == tdL.cl1 ? tdL.wins1++ : tdL.wins2++;

        memset(buffer, 0, sizeof(buffer));
        strcpy(buffer, "Current score: \n");
        
        strcat(buffer, tdL.name_cl1.c_str());
        strcat(buffer, itoa(tdL.wins1));

        strcat(buffer, " ");

        strcat(buffer, tdL.name_cl2.c_str());
        strcat(buffer, itoa(tdL.wins2));

        strcat(buffer, "\n");

        write(currentp, buffer, sizeof(buffer));
        write(otherp, buffer, sizeof(buffer)); 

        break;
        }

      strcpy(buffer, "1");
      write(otherp, buffer, sizeof(buffer));
      write(currentp, buffer, sizeof(buffer));


      strcpy(buffer, "Wait for the other player to make his move...");
      write(otherp, buffer, sizeof(buffer));


      if(currentp == player1)
        {
        strcpy(buffer, "You have the X pieces! Which column do you drop your piece into? [1-7] ");
        write(currentp, buffer, sizeof(buffer));
        }
        else
        {
        strcpy(buffer, "You have the O pieces! Which column do you drop your piece into? [1-7] ");
        write(currentp, buffer, sizeof(buffer));
        }

      while(1)
        {read (currentp, buffer, sizeof(buffer));
        
        if(!(buffer[0] >= '1' && buffer[0] <= '7'))
          {
          strcpy(buffer, "That is not a valid column! Try again [1-7] ");
          write(currentp, buffer, sizeof(buffer));
          continue;
          }

        played_column = buffer[0] - '0';
        if(matrix[1][played_column] != '-') //that column is full
          {strcpy(buffer, "That column is full! Try another one... ");
          write(currentp, buffer, sizeof(buffer));
          continue;
          }
        break;
        }

      strcpy(buffer, "Good move! ");
      write(currentp, buffer, sizeof(buffer));

      for(i = 1; i <= 6 && matrix[i][played_column] == '-'; i++);
      
      row = i-1;
      currentp == player1 ? matrix[row][played_column] = 'X' : matrix[row][played_column] = 'O';

      //test if game is over
      
      while(1)
      {
      for(i=row, j=played_column, nrc = 1; i>=1 && i<=6 && j>=1 && j<=7 && nrc <= 4; i++, j, nrc++)
        if(currentp == player1 ? matrix[i][j] != 'X' : matrix[i][j] != 'O')
          break;
      if(nrc==5) { ok=1; break;}

      for(i=row, j=played_column, nrc = 1; i>=1 && i<=6 && j>=1 && j<=7 && nrc <= 4; i++, j--, nrc++)
        if(currentp == player1 ? matrix[i][j] != 'X' : matrix[i][j] != 'O')
          break;
      if(nrc==5) {ok=2; break;}

      for(i=row, j=played_column, nrc = 1; i>=1 && i<=6 && j>=1 && j<=7 && nrc <= 4; i--, j++, nrc++)
        if(currentp == player1 ? matrix[i][j] != 'X' : matrix[i][j] != 'O')
          break;
      if(nrc==5) {ok=3; break;}
      
      for(i=row, j=played_column, nrc = 1; i>=1 && i<=6 && j>=1 && j<=7 && nrc <= 4; i--, j--, nrc++)
        if(currentp == player1 ? matrix[i][j] != 'X' : matrix[i][j] != 'O')
          break;
      if(nrc==5) {ok=4; break;}

      for(i=row, j=played_column, nrc = 1; i>=1 && i<=6 && j>=1 && j<=7 && nrc <= 4; i++, j++, nrc++)
        if(currentp == player1 ? matrix[i][j] != 'X' : matrix[i][j] != 'O')
          break;
      if(nrc==5) {ok=5; break;}

      for(i=row, j=played_column, nrc = 1; i>=1 && i<=6 && j>=1 && j<=7 && nrc <= 4; j--, nrc++)
        if(currentp == player1 ? matrix[i][j] != 'X' : matrix[i][j] != 'O')
          break;
      if(nrc==5) {ok=6; break;}

      for(i=row, j=played_column, nrc = 1; i>=1 && i<=6 && j>=1 && j<=7 && nrc <= 4; j++, nrc++)
        if(currentp == player1 ? matrix[i][j] != 'X' : matrix[i][j] != 'O')
          break;
      if(nrc==5) {ok=7; break;}

      break;
      }

        if(nrc == 5) // someone connected 4
        {
          memset(combo, 0, sizeof(combo));
          switch (ok)
          {
          case 1:
            {strcat (combo, strcatchar(row+64)); strcat (combo, itoa(played_column)); strcat(combo, ", "); strcat (combo, strcatchar(row+65)); strcat (combo, itoa(played_column)); strcat(combo, ", "); strcat (combo, strcatchar(row+66)); strcat (combo, itoa(played_column)); strcat(combo, ", "); strcat (combo, strcatchar(row+67)); strcat (combo, itoa(played_column)); strcat(combo, " ");}
            break;
          case 2:
            {strcat (combo, strcatchar(row+64)); strcat (combo, itoa(played_column)); strcat(combo, ", "); strcat (combo, strcatchar(row+65)); strcat (combo, itoa(played_column-1)); strcat(combo, ", "); strcat (combo, strcatchar(row+66)); strcat (combo, itoa(played_column-2)); strcat(combo, ", "); strcat (combo, strcatchar(row+67)); strcat (combo, itoa(played_column-3)); strcat(combo, " ");}
            break;
          case 3:
            {strcat (combo, strcatchar(row+64)); strcat (combo, itoa(played_column)); strcat(combo, ", "); strcat (combo, strcatchar(row+63)); strcat (combo, itoa(played_column+1)); strcat(combo, ", "); strcat (combo, strcatchar(row+62)); strcat (combo, itoa(played_column+2)); strcat(combo, ", "); strcat (combo, strcatchar(row+61)); strcat (combo, itoa(played_column+3)); strcat(combo, " ");}
            break;
          case 4:
            {strcat (combo, strcatchar(row+64)); strcat (combo, itoa(played_column)); strcat(combo, ", "); strcat (combo, strcatchar(row+63)); strcat (combo, itoa(played_column-1)); strcat(combo, ", "); strcat (combo, strcatchar(row+62)); strcat (combo, itoa(played_column-2)); strcat(combo, ", "); strcat (combo, strcatchar(row+61)); strcat (combo, itoa(played_column-3)); strcat(combo, " ");}
            break;
          case 5:
            {strcat (combo, strcatchar(row+64)); strcat (combo, itoa(played_column)); strcat(combo, ", "); strcat (combo, strcatchar(row+65)); strcat (combo, itoa(played_column+1)); strcat(combo, ", "); strcat (combo, strcatchar(row+66)); strcat (combo, itoa(played_column+2)); strcat(combo, ", "); strcat (combo, strcatchar(row+67)); strcat (combo, itoa(played_column+3)); strcat(combo, " ");}
            break;
          case 6:
            {strcat (combo, strcatchar(row+64)); strcat (combo, itoa(played_column)); strcat(combo, ", "); strcat (combo, strcatchar(row+64)); strcat (combo, itoa(played_column-1)); strcat(combo, ", "); strcat (combo, strcatchar(row+64)); strcat (combo, itoa(played_column-2)); strcat(combo, ", "); strcat (combo, strcatchar(row+64)); strcat (combo, itoa(played_column-3)); strcat(combo, " ");}
            break;
          case 7:
            {strcat (combo, strcatchar(row+64)); strcat (combo, itoa(played_column)); strcat(combo, ", "); strcat (combo, strcatchar(row+64)); strcat (combo, itoa(played_column+1)); strcat(combo, ", "); strcat (combo, strcatchar(row+64)); strcat (combo, itoa(played_column+2)); strcat(combo, ", "); strcat (combo, strcatchar(row+64)); strcat (combo, itoa(played_column+3)); strcat(combo, " ");}
            break;
          default:
            break;
          }
        }

    swap(currentp, otherp);
    }

  return winner;
}



void save_name(void *arg, int nr_client, struct thData &tdL)
{
  char buf[100];
  int i, cl;

  if (nr_client == 1)
      cl = tdL.cl1;
      else
      cl = tdL.cl2; 
	
  if (read (cl, buf, sizeof(buf)) <= 0)
			{
			  printf("[Thread %d]\n",tdL.idThread);
			  perror ("Eroare la read() de la client.\n");		
			}
	
	printf ("[Thread %d]Mesajul a fost receptionat...%s\n",tdL.idThread, buf);

  for(int i=0; i<=strlen(buf); i++)
    {
      if(buf[i]=='\n')
        buf[i]=' ';
    }

  if (nr_client == 1)
      tdL.name_cl1 = buf;
      else
      tdL.name_cl2 = buf;
}

char * itoa(int number)
{
  char * str = (char*)malloc(sizeof(char) * 10);
  int size = 0;
  bzero(str, sizeof(str));
  
  if(number==0) str[size++] = '0' + 0;
  while (number) {
    str[size++] = '0' + number % 10;
    number = number / 10;
  }

  char aux;
  for (int i = 0; i < size / 2; ++i)
  {
    aux = str[i];
    str[i] = str[size - 1 - i];
    str[size - 1 - i] = aux;
  }

  return str;
}

char * strcatchar(int value)
{
  char * str = (char*)malloc(sizeof(char) * 2);
  
  str[1] = '\0';
  str[0] = char(value); 

  return str;
}