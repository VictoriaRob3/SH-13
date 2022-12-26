#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#include <netdb.h>
#include <arpa/inet.h>


/**
 * \struct Str_t
 * \brief  structure qui fabrique un tab de4 elements qui garde les infos des clients qui se connnecte au jeu
 */


struct _client
{
        char ipAddress[40];
        int port;
        char name[40];
} tcpClients[4];


int nbClients=0;
int fsmServer=0; // variable d'etat du serveur

int deck[13]={0,1,2,3,4,5,6,7,8,9,10,11,12}; // packet de cartes. Il faudra les mélanger apres

int tableCartes[4][8]; // 4 lignes de 8 colonnes qui represente la matrice du jeu en cours

char *nomcartes[]=
{"Sebastian Moran", "irene Adler", "inspector Lestrade",
  "inspector Gregson", "inspector Baynes", "inspector Bradstreet",
  "inspector Hopkins", "Sherlock Holmes", "John Watson", "Mycroft Holmes",
  "Mrs. Hudson", "Mary Morstan", "James Moriarty"}; // nom des méchants


int joueurCourant; // joueur courant : le 1er connécté est le 1er a jouer

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

// apres ça, le deck sera mélangé
void melangerDeck()
{
        int i;
        int index1,index2,tmp;

        for (i=0;i<1000;i++)
        {
                index1=rand()%13; // entre 0 et 12
                index2=rand()%13;

                tmp=deck[index1];
                deck[index1]=deck[index2];
                deck[index2]=tmp; // on echange les 2 indices et on le fait 1000 fois
        }
}



// Cree la table
void createTable()
{
	// Le joueur 0 possede les cartes d'indice 0,1,2
	// Le joueur 1 possede les cartes d'indice 3,4,5 
	// Le joueur 2 possede les cartes d'indice 6,7,8 
	// Le joueur 3 possede les cartes d'indice 9,10,11 
	// Le coupable est la carte d'indice 12

	// permet de creer la table de jeu, apres le mélange

	int i,j,c;

	for (i=0;i<4;i++)
		for (j=0;j<8;j++)
			tableCartes[i][j]=0; // met tout a 0

	for (i=0;i<4;i++) // pour chaque joueur et les 3 cartes de chaque joeur
	{
		for (j=0;j<3;j++)
		{
			c=deck[i*3+j];
			switch (c)
			{
				case 0: // Sebastian Moran // le joeur i a recupéré sebastien moran
					tableCartes[i][7]++;
					tableCartes[i][2]++;
					break;
				case 1: // Irene Adler
					tableCartes[i][7]++;
					tableCartes[i][1]++;
					tableCartes[i][5]++;
					break;
				case 2: // Inspector Lestrade
					tableCartes[i][3]++;
					tableCartes[i][6]++;
					tableCartes[i][4]++;
					break;
				case 3: // Inspector Gregson 
					tableCartes[i][3]++;
					tableCartes[i][2]++;
					tableCartes[i][4]++;
					break;
				case 4: // Inspector Baynes 
					tableCartes[i][3]++;
					tableCartes[i][1]++;
					break;
				case 5: // Inspector Bradstreet 
					tableCartes[i][3]++;
					tableCartes[i][2]++;
					break;
				case 6: // Inspector Hopkins 
					tableCartes[i][3]++;
					tableCartes[i][0]++;
					tableCartes[i][6]++;
					break;
				case 7: // Sherlock Holmes 
					tableCartes[i][0]++;
					tableCartes[i][1]++;
					tableCartes[i][2]++;
					break;
				case 8: // John Watson 
					tableCartes[i][0]++;
					tableCartes[i][6]++;
					tableCartes[i][2]++;
					break;
				case 9: // Mycroft Holmes 
					tableCartes[i][0]++;
					tableCartes[i][1]++;
					tableCartes[i][4]++;
					break;
				case 10: // Mrs. Hudson 
					tableCartes[i][0]++;
					tableCartes[i][5]++;
					break;
				case 11: // Mary Morstan 
					tableCartes[i][4]++;
					tableCartes[i][5]++;
					break;
				case 12: // James Moriarty 
					tableCartes[i][7]++;
					tableCartes[i][1]++;
					break;
			}
		}
	}
} 

// a la fin, si le tableau rempli, le jeu est fini

void printDeck()
{
	int i, j;

	for (i = 0; i < 13; i++)
		printf("%d %s\n", deck[i], nomcartes[deck[i]]);

	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 8; j++)
			printf("%2.2d ", tableCartes[i][j]);
		puts("");
	}
}

void printClients()
{
	int i;

	for (i = 0; i < nbClients; i++)
		printf("%d: %s %5.5d %s\n", i, tcpClients[i].ipAddress,
			   tcpClients[i].port,
			   tcpClients[i].name);
}

int findClientByName(char *name)
{
	int i;

	for (i = 0; i < nbClients; i++)
		if (strcmp(tcpClients[i].name, name) == 0)
			return i;
	return -1;
}

// envoie un messagge au client

void sendMessageToClient(char *clientip,int clientport,char *mess)
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[256];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    server = gethostbyname(clientip);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(clientport);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        {
                printf("ERROR connecting\n");
                exit(1);
        }

        sprintf(buffer,"%s\n",mess);
        n = write(sockfd,buffer,strlen(buffer));
		printf("envoye a %s de port %d: %s\n",clientip,clientport,buffer);

    close(sockfd);
}


// envoie message a tous les clients
void broadcastMessage(char *mess)
{
	int i;

	for (i = 0; i < nbClients; i++)
				sendMessageToClient(tcpClients[i].ipAddress,
									tcpClients[i].port,
									mess);
}

int main(int argc, char *argv[])
{

	int sockfd, newsockfd, portno;
	socklen_t clilen;
	char buffer[256];
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	int i;

	char com;
	char clientIpAddress[256], clientName[256];
	int clientPort;
	int id;
	char reply[256];

	if (argc < 2)
	{
				fprintf(stderr, "ERROR, no port provided\n");
				exit(1);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);


	if (sockfd < 0)
				error("ERROR opening socket");
	
	bzero((char *)&serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	
	if (bind(sockfd, (struct sockaddr *)&serv_addr,
			 sizeof(serv_addr)) < 0)
				error("ERROR on binding");
	listen(sockfd, 5);
	clilen = sizeof(cli_addr);


	printf("DEBUT DU JEU SH13\n");
	printDeck();
	melangerDeck();
	createTable();
	printDeck();
	
	joueurCourant = 0;

	for (i = 0; i < 4; i++)
	{
				strcpy(tcpClients[i].ipAddress, "localhost"); // on initialise les clients
				tcpClients[i].port = -1;
				strcpy(tcpClients[i].name, "-");
	}

	while (1) // on se bloque sur le accept
	{
				newsockfd = accept(sockfd,
								   (struct sockaddr *)&cli_addr,
								   &clilen);
				if (newsockfd < 0)
			error("ERROR on accept");

				bzero(buffer, 256);
				n = read(newsockfd, buffer, 255);
				if (n < 0)
			error("ERROR reading from socket");

				printf("Received packet from %s:%d\nData: [%s]\n\n",
					   inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), buffer);

				// j'ai reçu un message :

				if (fsmServer == 0) // si j'ai 0, alors j'attends encore d'autres joueues
				{
			switch (buffer[0])
			{
			case 'C': // si grand C, messsage de 1ere cinnexion
					sscanf(buffer, "%c %s %d %s", &com, clientIpAddress, &clientPort, clientName);
					printf("COM=%c ipAddress=%s port=%d name=%s\n", com, clientIpAddress, clientPort, clientName);

					// fsmServer==0 alors j'attends les connexions de tous les joueurs
					strcpy(tcpClients[nbClients].ipAddress, clientIpAddress);
					tcpClients[nbClients].port = clientPort;
					strcpy(tcpClients[nbClients].name, clientName);
					nbClients++; // le client a bien été ajouté

					printClients(); // affiche la table des clients

					// rechercher l'id du joueur qui vient de se connecter

					id = findClientByName(clientName); // numero du joueur dont je reçois le param
					printf("id=%d\n", id);

					// lui envoyer un message personnel pour lui communiquer son id

					sprintf(reply, "I %d", id);
					sendMessageToClient(tcpClients[id].ipAddress,
										tcpClients[id].port,
										reply); // on envoie I id au client concerné

					// Envoyer un message broadcast pour communiquer a tout le monde la liste des joueurs actuellement
					// connectes

					sprintf(reply, "L %s %s %s %s", tcpClients[0].name, tcpClients[1].name, tcpClients[2].name, tcpClients[3].name);
					broadcastMessage(reply); // tout le monde avertit

					// Si le nombre de joueurs atteint 4, alors on peut lancer le jeu

					if (nbClients == 4)
					{
						int cartes_distribuees = 0;
						// On envoie ses cartes au joueur 0, ainsi que la ligne qui lui correspond dans tableCartes
						// RAJOUTER DU CODE ICI

						// sprintf(reply,"D %s %s %s %s", );
						// sendMessageToClient(tcpClients[id].ipAddress,tcpClients[id].port,reply);

						// sprintf(reply,"D %s %s %s %s", );
						// sendMessageToClient(tcpClients[id].ipAddress,tcpClients[id].port,reply);

						// On envoie ses cartes au joueur 1, ainsi que la ligne qui lui correspond dans tableCartes
						// RAJOUTER DU CODE ICI

						// On envoie ses cartes au joueur 2, ainsi que la ligne qui lui correspond dans tableCartes
						// RAJOUTER DU CODE ICI

						// On envoie ses cartes au joueur 3, ainsi que la ligne qui lui correspond dans tableCartes
						// RAJOUTER DU CODE ICI

						// On envoie enfin un message a tout le monde pour definir qui est le joueur courant=0
						// RAJOUTER DU CODE ICI

						fsmServer = 1;
					}
					break;
			}
				}
				else if (fsmServer == 1) // cas ou tout le monde est connecté : donc message de jeu
				{
			switch (buffer[0])
			{
			case 'G':
					// RAJOUTER DU CODE ICI
					break;
			case 'O':
					// RAJOUTER DU CODE ICI
					break;
			case 'S':
					// RAJOUTER DU CODE ICI
					break;
			default:
					break;
			}
				}
				close(newsockfd);
	}
	close(sockfd);
	return 0;
}