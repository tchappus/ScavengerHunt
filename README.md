# ScavENGer Hunt

![Western Engineering](./Engineer_Stacked_PurpleGrey.png)

This project was done for SE3313: Operating Systems in December 2017

### Group Info: <br />
Trent Chappus - 250845587 <br />
Ashley Ottogalli - 250845708 <br />
Connor McCauley - 250865901 <br />
Matthew Price - 250752191 <br />
Rachel Vanderloop - 250798250 <br />

## OVERVIEW
Our group’s creation, ScavENGer Hunt, can be described as a fun, addictive, and thrilling multiplayer game for Android. The rules of the game are very simple: be the first person in the match to take a photo of the displayed word. The winner of the game will be determined by whoever has the most wins after 3 matches are successfully completed. 

## FLOW OF EVENTS
Server is awaiting request from clients
Player 1 opens game on their Android device
Player 1’s device sends a start game request to the server
The server initializes a game instance and responds to the device with a JSON object containing the game’s information (game ID, status, current word, and starting time).

Player 2 opens the game on their Android device and also sends a start game request. 

The server then searches through all of the ongoing games for a pending game. Once one is found, it will send a JSON object containing the game’s information (ex: the game Id, status, the current word, and starting time). 

Both client side applications will then periodically (every second) make requests to the server with the game ID for information on the current game. The server responds with the current game data in a JSON. The client side then updates the player’s User Interface. 

When a player captures an image, it will be sent to IBM Watson Visual Recognition to classify the image. If the classification of the sent image matches the current word, the client will respond with sending a “won game” request to the server.

Once the server receives the “won game” request, it instantly refreshes the game with a new word and starting time.

If a general get request is made by the server, and the match has been going on for more than 60 seconds, the server will refresh the game session with a new word and starting time.

Once three rounds have happened, the server terminates all of the game instances. Any future requests made for that game will be responded to with a “NOGAMEFOUND” status. The client deals with this status accordingly (i.e: starts a new game).

## SPECIFICATIONS
### Server
The server is an Amazon Web Services (AWS) Ubuntu instance. The server was written in C++ and compiled using GCC. As the AWS Ubuntu instance is accessible over the Internet by its IP address, it is an ideal solution for communicating game information to Android clients. 
We used the following classes for the implementation of the server: Socket, SocketServer, Shared, and ThreadSem. Additionally, std::thread is used for creating threads.

When the server receives a new request, it will respond by generating a new thread. Through the creation of new threads, the server is virtually always available to accept new requests.

Any current generated games are stored in an std::vector in a shared memory object. 

Two semaphores are used in the server application: one is used to control access to the vector, the other as a pseudo-event system, which will signal a waiting player that another player has joined the match and the game is ready to start. 

Upon shutdown, Socket::Shutdown() is called and all running threads are joined. This provides graceful termination.

### Client
The client application is written as an Android application written in Java and developed using Android Studio. Android devices are optimal for this game due to onboard cameras and easy development with Java (which supports IBM Watson and Socket development).

Two implementations of AsyncTask are used: one to make requests to the server, and one to make requests to IBM Watson Visual Recognition.
## DESIGN CHANGES
The design as described in the Vision Document and the implemented design are slightly different. The server was implemented as stateless (does not store any information about the clients). The server solely provides information on a game based on its ID. Threads are created to handle every request and are terminated after the request is replied to. Therefore, it does not to keep track of players or matches. For demonstration purposes, the gameplay was designed with only two players in a match. Additionally, the game will go on until three wins occur, rather than having a static 10 matches. These simplifications were made necessary for the demonstration of the game, to be extensible and modifiable in a timely manner.
## LESSONS LEARNED
- How to use IBM Watson to classify images
- How to integrate an API like IBM Watson in an Android application
- How to make requests and take responses from our API, using sockets in Java.
- How to effectively use Android’s AsyncTask interface
- How to effectively use Semaphores and Threads in C++
- How to take requests and send responses to a client, from a Ubuntu instance using C++

## DISTRIBUTION OF TASKS
Trent Chappus: Primary implementation of backend code. Designed main thread procedure to accept connections from a socket and spawn a new thread to respond to the request. Designed request thread procedure to determine what the request was and what to reply with. In doing so, implemented an API interface for the client. Made use of a shared memory object to store game information and implemented access control using a semaphore. Implemented matchmaking using a semaphore.

Ashley Ottogalli: Assisted with frontend Android development. Implemented an access point for the app to connect to Watson with a Bitmap received from the camera app. Got the response from Watson to return a ClassifiedImages object that could be used client-side to update game status. Developed the layouts for the MainActivity (Start screen) and the InGameActivity. 

Rachel Vanderloop: Assisted with backend server development. Helped with research related to C++ syntax and standard libraries. Assisted in overall API design to ensure proper interface was provided for frontend client.

Connor McCauley: Assisted with backend server development. Helped with research related to C++ syntax and standard libraries. Responsible for GitHub Repository management tasks, such as merging and pushing branches and ensuring proper version control of project resources.

Matthew Price: Primary implementation of frontend Android client. Connected the Android app to the server using a socket connection. Parsed the ClassifiedImages objects to receive the different Watson responses as an ArrayList of strings. Separated the string sent from the server into a JSON which would be split and used the information to update the game status. Compared image results with server information to sent appropriate information to server upon status change (Game completion, user exiting, requesting a new game start). Updated the components of the layout with the appropriate information received from the user inputs, Watson responses and server messages. 

## DISCUSSION
Overall, this project was an excellent opportunity towards the advancement of our knowledge in C++, Android Studio development, and server-client architectures. Additionaly, it introduced us to using a remote Ubuntu instance that used AWS. One of the main benefits of this project was the solidarity that it gave to the topics that were discussed in class. The open-endedness of the project allowed us to implement a unique and creative idea that kept us engaged. In addition, the development of our application taught us how to complete a large project as a team, and forced us to work on our task delegation techniques. Lastly, this project provided an excellent environment to work in a software team developing challenging ideas in a similar fashion to graduated software teams (i.e: on the workforce). Each of us will be able to carry a greater understanding of how to work around everyone’s unique skill set in a workplace environment. We are thankful for the opportunity. 

