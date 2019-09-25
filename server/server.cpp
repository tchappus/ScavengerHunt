#include "thread.h"
#include "socketserver.h"
#include <stdlib.h>
#include <time.h>
#include <list>
#include <vector>
#include <string>
#include <locale>
#include "SharedObject.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "SharedStuff.h"

using namespace Sync;

void threadProcedure(Socket conn)
{
    //get reference to shared memory
    Shared<SharedStuff> sharedMem("run");

    //read sent message from client
    ByteArray receivedMsg;
    conn.Read(receivedMsg);
    std::string msgString = receivedMsg.ToString();
    std::cout << "received: " << msgString << std::endl;

    //if client sent nothing
    if (msgString == "")
    {
        //send error message
        std::stringstream response;
        response << "{ \"ID\": \"" << 000000 << "\", \"Status\": \""
                 << "NOGAMEFOUND"
                 << "\", \"CurrentWord\": \""
                 << "ERROR"
                 << "\", \"TimeStarted\": \"" << 0 << "\"}";
        std::cout << "Invalid request made." << std::endl;
        std::cout << response.str() << std::endl;
        ByteArray res(response.str());
        conn.Write(res);
        conn.Close();
        return;
    }

    //format request
    std::string header;
    std::string route;
    bool gotHeader = false;

    for (int i = 0; i < msgString.length(); i++)
    {
        if (msgString[i] == ' ')
        {
            if (!gotHeader)
            {
                gotHeader = true;
            }
            else
            {
                break;
            }
        }
        else
        {
            if (!gotHeader)
            {
                header.push_back(msgString[i]);
            }
            else
            {
                route.push_back(msgString[i]);
            }
        }
    }

    //whether or not we've sent a reply
    bool sentAthing = false;

    //GET requests
    if (header == "GET")
    {

        //get game info
        if (route.substr(0, 6) == "/game/")
        {

            //get game ID requested
            std::string gameID = route.substr(6, 6);
            std::cout << "game info requested for " << gameID << std::endl;

            //use semaphore to control access to the vector
            (*sharedMem->currentgames_sem).Wait();
            //start of critical section

            //search for game requested
            for (int i = 0; i < sharedMem->currentgames.size(); i++)
            {
                if ((*sharedMem->currentgames[i]).ID == gameID)
                {

                    //restart match if it's lasted over 60 seconds
                    if ((time(0) - (*sharedMem->currentgames[i]).timeStarted) > 60)
                    {
                        std::cout << "game starting over " << gameID << std::endl;
                        int index = rand() % (*sharedMem->currentgames[i]).words.size();
                        (*sharedMem->currentgames[i]).word = (*sharedMem->currentgames[i]).words[index];
                        (*sharedMem->currentgames[i]).timeStarted = time(0);
                    }

                    //finish game if three matches have gone
                    if ((*sharedMem->currentgames[i]).numMatches > 1)
                    {
                        (*sharedMem->currentgames[i]).status = "DONE";
                    }

                    //send info about game
                    std::stringstream response;
                    response << "{ \"ID\": \"" << (*sharedMem->currentgames[i]).ID << "\", \"Status\": \"" << (*sharedMem->currentgames[i]).status << "\", \"CurrentWord\": \"" << (*sharedMem->currentgames[i]).word << "\", \"TimeStarted\": \"" << (*sharedMem->currentgames[i]).timeStarted << "\"}";
                    std::cout << "sending game info response" << std::endl;
                    std::cout << response.str() << std::endl;
                    ByteArray res(response.str());

                    //end of critical section
                    (*sharedMem->currentgames_sem).Signal();

                    //send and close connection
                    conn.Write(res);
                    conn.Close();
                    sentAthing = true;

                    //if game is done, delete it
                    if ((*sharedMem->currentgames[i]).status == "DONE")
                    {
                        delete sharedMem->currentgames[i];
                        sharedMem->currentgames.erase(sharedMem->currentgames.begin() + i);
                    }

                    break;
                }
            }

            //if no game was found, send an error
            if (!sentAthing)
            {
                //end of critical section
                (*sharedMem->currentgames_sem).Signal();

                //send response
                std::stringstream response;
                response << "{ \"ID\": \"" << 000000 << "\", \"Status\": \""
                         << "NOGAMEFOUND"
                         << "\", \"CurrentWord\": \""
                         << "ERROR"
                         << "\", \"TimeStarted\": \"" << 0 << "\"}";
                std::cout << "Invalid request made for nonexistent game" << std::endl;
                std::cout << response.str() << std::endl;
                ByteArray res(response.str());
                conn.Write(res);
                conn.Close();
            }

        }
        else if (route.substr(0, 11) == "/startgame/")
        {
            //client wants to start a new game
            std::cout << "starting game" << std::endl;

            //create a new game if there aren't any
            if (sharedMem->currentgames.size() == 0)
            {
                sharedMem->currentgames.push_back(new Game());
            }

            //control access to vector
            (*sharedMem->currentgames_sem).Wait();
            //start of critical section

            //search for current games
            for (int i = 0; i < sharedMem->currentgames.size(); i++)
            {
                //if game is waiting for more players, lets put client in it
                if ((*sharedMem->currentgames[i]).status == "WAITING")
                {
                    //make them a name
                    int size = (*sharedMem->currentgames[i]).players.size();
                    std::stringstream name;
                    name << "Player" << (size + 1);
                    (*sharedMem->currentgames[i]).players.push_back(name.str());

                    //if there are 2 players, we can start the game
                    if ((*sharedMem->currentgames[i]).players.size() == 2)
                    {
                        //set game attributes
                        (*sharedMem->currentgames[i]).status = "ONGOING";
                        (*sharedMem->currentgames[i]).timeStarted = time(0);

                        //unblock waiting players
                        (*sharedMem->currentgames[i]).waitingSem.Signal();

                        //end of critical section
                        (*sharedMem->currentgames_sem).Signal();

                        //send response of game details in JSON format
                        std::stringstream response;
                        std::cout << "sending start game response" << std::endl;
                        response << "{ \"ID\": \"" << (*sharedMem->currentgames[i]).ID << "\", \"Status\": \"" << (*sharedMem->currentgames[i]).status << "\", \"CurrentWord\": \"" << (*sharedMem->currentgames[i]).word << "\", \"TimeStarted\": \"" << (*sharedMem->currentgames[i]).timeStarted << "\"}";
                        std::cout << response.str() << std::endl;
                        ByteArray res(response.str());
                        conn.Write(res);
                        conn.Close();
                        sentAthing = true;

                        break;
                    }
                    else
                    {
                        //we're waiting for more players
                        std::cout << "waiting for more players..." << std::endl;
                        
                        //end of critical section
                        (*sharedMem->currentgames_sem).Signal();

                        //block until more players join
                        (*sharedMem->currentgames[i]).waitingSem.Wait();

                        //we're starting the game
                        std::stringstream response;
                        response << "{ \"ID\": \"" << (*sharedMem->currentgames[i]).ID << "\", \"Status\": \"" << (*sharedMem->currentgames[i]).status << "\", \"CurrentWord\": \"" << (*sharedMem->currentgames[i]).word << "\", \"TimeStarted\": \"" << (*sharedMem->currentgames[i]).timeStarted << "\"}";
                        std::cout << response.str() << std::endl;
                        ByteArray res(response.str());
                        conn.Write(res);
                        conn.Close();
                        sentAthing = true;
                        break;
                    }
                }
                else if (i == sharedMem->currentgames.size() - 1)
                {
                    //no waiting games, so make one and start over
                    std::cout << "no waiting games, making one" << std::endl;
                    sharedMem->currentgames.push_back(new Game());
                    i = 0;
                }
            }

            if (!sentAthing)
            {
                conn.Close();
            }

            //end of critical section
            (*sharedMem->currentgames_sem).Signal();
        }
        else if (route.substr(0, 9) == "/wongame/")
        {
            //client has won a game
            std::string gameID = route.substr(9, 6);
            std::cout << "request to win game " << gameID << std::endl;

            (*sharedMem->currentgames_sem).Wait();
            //start of critical section

            //search for game ID
            for (int i = 0; i < sharedMem->currentgames.size(); i++)
            {
                if ((*sharedMem->currentgames[i]).ID == gameID)
                {
                    //get a new word, start time over, and add to number of matches
                    int index = rand() % (*sharedMem->currentgames[i]).words.size();
                    (*sharedMem->currentgames[i]).word = (*sharedMem->currentgames[i]).words[index];
                    (*sharedMem->currentgames[i]).timeStarted = time(0);
                    (*sharedMem->currentgames[i]).numMatches++;

                    //send a response according to situation
                    std::stringstream response;
                    if ((*sharedMem->currentgames[i]).numMatches > 1)
                    {
                        //game is done
                        (*sharedMem->currentgames[i]).status = "DONE";
                        response << "{ \"ID\": \"" << (*sharedMem->currentgames[i]).ID << "\", \"Status\": \"" << (*sharedMem->currentgames[i]).status << "\", \"CurrentWord\": \"" << (*sharedMem->currentgames[i]).word << "\", \"TimeStarted\": \"" << (*sharedMem->currentgames[i]).timeStarted << "\"}";
                        delete sharedMem->currentgames[i];
                        sharedMem->currentgames.erase(sharedMem->currentgames.begin() + i);
                    }
                    else
                    {
                        //next round has started
                        response << "{ \"ID\": \"" << (*sharedMem->currentgames[i]).ID << "\", \"Status\": \"" << (*sharedMem->currentgames[i]).status << "\", \"CurrentWord\": \"" << (*sharedMem->currentgames[i]).word << "\", \"TimeStarted\": \"" << (*sharedMem->currentgames[i]).timeStarted << "\"}";
                    }
                    //end of critical section
                    (*sharedMem->currentgames_sem).Signal();

                    //send response
                    std::cout << "sending game info response" << std::endl;
                    std::cout << response.str() << std::endl;
                    ByteArray res(response.str());

                    conn.Write(res);
                    conn.Close();
                    sentAthing = true;

                    break;
                }
            }
            //if we haven't sent anything, send error (game was not found)
            if (!sentAthing)
            {
                (*sharedMem->currentgames_sem).Signal();
                std::stringstream response;
                response << "{ \"ID\": \"" << 000000 << "\", \"Status\": \""
                         << "NOGAMEFOUND"
                         << "\", \"CurrentWord\": \""
                         << "ERROR"
                         << "\", \"TimeStarted\": \"" << 0 << "\"}";
                std::cout << "Invalid request made for nonexistent game" << std::endl;
                std::cout << response.str() << std::endl;
                ByteArray res(response.str());
                conn.Write(res);
                conn.Close();
            }
        }
        else if (route.substr(0, 9) == "/endgame/")
        {
            //client has exited game
            std::string gameID = route.substr(9, 6);
            std::cout << "client wishes to end game " << gameID << std::endl;

            (*sharedMem->currentgames_sem).Wait();
            //start of critical section

            for (int i = 0; i < sharedMem->currentgames.size(); i++)
            {
                if ((*sharedMem->currentgames[i]).ID == gameID)
                {
                    //delete the game
                    delete sharedMem->currentgames[i];
                    sharedMem->currentgames.erase(sharedMem->currentgames.begin() + i);

                    //send response that the game has ended
                    std::stringstream response;
                    std::cout << "End game response sending:" << std::endl;
                    response << "{ \"ID\": \"" << 000000 << "\", \"Status\": \""
                             << "DONE"
                             << "\", \"CurrentWord\": \""
                             << "ERROR"
                             << "\", \"TimeStarted\": \"" << 0 << "\"}";
                    std::cout << response.str() << std::endl;
                    ByteArray res(response.str());
                    conn.Write(res);
                    conn.Close();
                    sentAthing = true;

                    break;
                }
            }

            //if game wasn't found, send an error
            if (!sentAthing)
            {
                std::stringstream response;
                response << "{ \"ID\": \"" << 000000 << "\", \"Status\": \""
                         << "NOGAMEFOUND"
                         << "\", \"CurrentWord\": \""
                         << "ERROR"
                         << "\", \"TimeStarted\": \"" << 0 << "\"}";
                std::cout << "Invalid request made for nonexistent game" << std::endl;
                std::cout << response.str() << std::endl;
                ByteArray res(response.str());
                conn.Write(res);
                conn.Close();
            }

            //end of critical section
            (*sharedMem->currentgames_sem).Signal();
        }
    }
}

void serverThread()
{

    //vector of created threads
    std::vector<std::thread *> runningThreads;

    //get reference to shared memory
    Shared<SharedStuff> sharedMem("run");

    //start server on port 2010
    sharedMem->socketServer = new SocketServer(2010);

    //accept connections and respond to them using a new thread
    while (sharedMem->run)
    {
        Socket conn = (*sharedMem->socketServer).Accept();
        runningThreads.push_back(new std::thread(threadProcedure, conn));
    }

    //finish up requests and tidy up memory
    for (int i = 0; i < runningThreads.size(); i++)
    {
        (*runningThreads[i]).join();
        delete runningThreads[i];
        runningThreads.erase(runningThreads.begin() + i);
    }
    std::vector<std::thread *>().swap(runningThreads);
}

int main(void)
{
    std::cout << "ScavENGer Hunt Server v1.0" << std::endl;

    //get reference to shared memory
    Shared<SharedStuff> sharedMem("run", true);

    sharedMem->run = true;
    sharedMem->currentgames_sem = new ThreadSem(1);

    //create main thread
    std::thread mainThread(serverThread);

    std::cout << "Type anything to quit." << std::endl;

    std::string input;
    std::cin >> input;
    std::cout << "ending..." << std::endl;

    //set shared bool to false and join threads
    sharedMem->run = false;
    (*sharedMem->socketServer).Shutdown();
    mainThread.join();

    //clearing allocated memory
    for (int i = 0; i < (sharedMem->currentgames).size(); i++)
    {
        delete (sharedMem->currentgames[i]);
    }
    std::vector<Game *>().swap(sharedMem->currentgames);
    delete (sharedMem->socketServer);
    delete (sharedMem->currentgames_sem);

    return 0;
}
