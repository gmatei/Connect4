#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
int port;


int main (int argc, char *argv[])
{
  int sd;			// descriptorul de socket
  struct sockaddr_in server;	// structura folosita pentru conectare 
  char buf[100]; // mesajul trimis

  /* exista toate argumentele in linia de comanda? */
  if (argc != 3)
    {
      printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
      return -1;
    }

  /* stabilim portul */
  port = atoi (argv[2]);

  /* cream socketul */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("Eroare la socket().\n");
      return errno;
    }

  /* umplem structura folosita pentru realizarea conexiunii cu serverul */
  /* familia socket-ului */
  server.sin_family = AF_INET;
  /* adresa IP a serverului */
  server.sin_addr.s_addr = inet_addr(argv[1]);
  /* portul de conectare */
  server.sin_port = htons (port);
  
  /* ne conectam la server */
  if (connect (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
      perror ("[client]Eroare la connect().\n");
      return errno;
    }

  /* citirea mesajului */
  printf ("[client] Welcome to Connect4! What's your name? ");
  fflush (stdout);
  
  read (0, buf, sizeof(buf));

  printf("[client] Hello %s Wait for the other player to connect!\n", buf);
  fflush (stdout);

  /* trimiterea mesajului la server */
  if (write (sd, buf, sizeof(buf)) <= 0)
    {
      perror ("[client] Eroare la write() spre server.\n");
      return errno;
    }
  
  while(1)
    {
      while(1)
      {
      printf ("[client]\n");
      //citit si afisat matrice
      for(int i=0; i<=6; i++)
      {
        bzero (buf, sizeof (buf));
        read (sd, buf, sizeof(buf));
        printf ("%s\n", buf);
        
      }

      //citit Waitforplayer/makeyourmove    sau   gameover
      read (sd, buf, sizeof(buf));
      if(buf[0] == '0')
        {
          read (sd, buf, sizeof(buf));
          printf ("[client] %s\n", buf);
          read (sd, buf, sizeof(buf));
          printf ("[client] %s\n", buf);
          break;
        }
        else
        {
          read (sd, buf, sizeof(buf));
          printf ("[client] %s\n", buf);

          if(buf[0] == 'Y')
            {
              while(1)
              {
                read (0, buf, sizeof(buf)); 
                write (sd, buf, sizeof(buf));
                read (sd, buf, sizeof(buf));

                if(buf[0] == 'T')
                  {printf ("[client] %s\n", buf); continue;}
                  else
                  break;
                  
              }
            }
        }
      } 
      //game is over

      read (sd, buf, sizeof(buf));
      printf ("[client] %s\n", buf);      //would you like another game

      read (0, buf, sizeof(buf)); 
      write (sd, buf, sizeof(buf));      // yes or no

      if(buf[0] == 'N')
        {
          printf ("[client] Thank you for playing! You will be disconnected from the server...\n ");
          fflush (stdout);
          break;
        }

      //answered yes

      read (sd, buf, sizeof(buf));      

      if(buf[0] == '0')            // reads the other player disconnected
        {
          printf ("[client] The other player left the game! You will be disconnected from the server...\n ");
          fflush (stdout);
          break;
        }

      printf ("[client]%s\n", buf);  //reads lets go again

    }

 
  /* inchidem conexiunea, am terminat */
  close (sd);
}