package ca.uwo.eng.se3313.project.scavenggerhunt;

import android.Manifest;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Bundle;
import android.os.CountDownTimer;
import android.os.Environment;
import android.provider.MediaStore;
import android.provider.OpenableColumns;
import android.support.annotation.RequiresApi;
import android.support.v4.content.FileProvider;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import com.google.gson.Gson;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonParser;
import com.ibm.watson.developer_cloud.visual_recognition.v3.VisualRecognition;
import com.ibm.watson.developer_cloud.visual_recognition.v3.model.ClassResult;
import com.ibm.watson.developer_cloud.visual_recognition.v3.model.ClassifiedImage;
import com.ibm.watson.developer_cloud.visual_recognition.v3.model.ClassifiedImages;
import com.ibm.watson.developer_cloud.visual_recognition.v3.model.ClassifierResult;
import com.ibm.watson.developer_cloud.visual_recognition.v3.model.ClassifyOptions;


import java.io.BufferedReader;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;


public class InGameActivity extends AppCompatActivity {

    //Declaring all variables that will be used in the class
    static final int REQUEST_IMAGE_CAPTURE = 1;
    private TextView wordView;
    private TextView status;
    private TextView scoreShown;
    private TextView timeLeft;
    private int score;
    private String ID;
    private String Status;
    private String CurrentWord;
    private String TimeStarted;
    private int winAmount;
    private int TimeInit;
    private long timeRemain;
    private CountDownTimer countDownTimer;
    private Boolean live = false;
    private Boolean running;
    private String oldID;
    private Boolean exiting;
    private Button btnCamera;
    private Button exitBtn;
    String mCurrentPhotoPath;
    VisualRecognition watty;


    public InGameActivity() throws IOException {
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.ingame_view);
        //declare the initial states for game begin
        RequestTask requestTask = new RequestTask();
        requestTask.execute("GET /startgame/"); //send message to server requesting a game
        exiting = false; // Make it so the game cannot send a finish game request if the game hasn't began.
        score = 0;
        winAmount = 2;
        running = true;
        scoreShown = findViewById(R.id.wordText3);
        wordView = findViewById(R.id.wordText);
        status = findViewById(R.id.wordText2);
        timeLeft = findViewById(R.id.wordText4);
        btnCamera = findViewById(R.id.btnCamera);
        exitBtn = findViewById(R.id.button);
        wordView.setText("Waiting..."); // Set text to show user is waiting to connect



        countDownTimer = new CountDownTimer(90000, 1000) {

            @RequiresApi(api = Build.VERSION_CODES.O)
            public void onTick(long millisUntilFinished) {
                timeRemain = 84 - ((System.currentTimeMillis()/1000) - TimeInit); // Using the time difference between the server/the systems we are using
                if (timeRemain < 1 && running){ //Accounting for time differential on different systems
                    timeRemain = 0;
                    status.setText("");
                }
                timeLeft.setText(String.valueOf(timeRemain));
                live = true; //Show that the game is live on every tick

                if (running == true && !ID.equals(oldID)){ //Check to make sure the game is not requesting an old game ID
                RequestTask requestTask = new RequestTask();
                requestTask.execute("GET /game/" + ID);
                }
                else if (running == true && ID.equals(oldID)){ //Set state to waiting while updating information to new game ID
                    wordView.setText("Waiting...");
                }
                else{ // present the results of the game to the users upon completion
                    wordView.setText("Done");
                    if (score > winAmount || scoreShown.getText().equals(Integer.toString(winAmount)) || scoreShown.getText().equals(Integer.toString(winAmount+1))){ //Check how many wins
                        status.setText("You win");
                    }
                    else{
                        status.setText("Game over");
                    }
                    timeLeft.setText("0"); //Set the time to 0 upon finish
                    btnCamera.setText("REPLAY"); //Switch the button to be purpose as a replay functionality
                }


            }

            public void onFinish() {
                countDownTimer.start(); //When the timer finishes, restart it.

            }
        };



        watty = new VisualRecognition(
                VisualRecognition.VERSION_DATE_2016_05_20
        );
        watty.setApiKey("0d1c6b7800efff5a855e45c0638cfaa778d186ca"); // Initialize our variables to connect to Watson using our API key
    }
    class RequestTask extends AsyncTask<String, Void, String> { //Connect to the server using sockets

        private Exception exception;

        protected String doInBackground(String...request) {
            try {
                Socket echoSocket = new Socket("54.167.215.132", 2010);

                PrintWriter out =
                        new PrintWriter(echoSocket.getOutputStream(), true);

                BufferedReader in =
                        new BufferedReader(
                                new InputStreamReader(echoSocket.getInputStream()));


                out.println(request[0]);

                // Consume the initial welcoming messages from the server
                return in.readLine();

            } catch (Exception e) {
                this.exception = e;
                System.out.println(this.exception);
                return null;
            }
        }

        protected void onPostExecute(String result) { //Immediate reacting to server response
            exiting = true; //Setting the possibility to exit the game to true based on game beginning
            if (result!= null){ //Checking to see if the object sent from the server is null to avoid crashes in case of error.
                if (running == true) { //If a game instance is currently running
                    if (ID == oldID){
                        btnCamera.setText("Camera"); //If the current game ID is the old ID, immediately after reset, set camera to Camera functionality
                        status.setText(""); // Remove old status
                    }
                    Gson gson = new Gson();
                    JsonParser json = new JsonParser(); //Use google's JsonParser to change a string into a JSON to be sorted through
                    try {
                        JsonElement jsonElement = json.parse(result); //Parse the string into a JsonElement
                        JsonObject jsonObject = jsonElement.getAsJsonObject(); //Get a JsonObject from that Element
                        if (!jsonObject.get("ID").getAsString().equals("0")){ //If game ID is not 0, therefore exists
                            ID = jsonObject.get("ID").getAsString(); //Set ID variable to the current received GAME ID
                        }
                        Status = jsonObject.get("Status").getAsString(); // Set Status to the received Status of game
                        if (!Status.equals("NOGAMEFOUND")) {
                            CurrentWord = jsonObject.get("CurrentWord").getAsString(); //If a game exists, get the current word and set it for use
                        }
                        TimeStarted = jsonObject.get("TimeStarted").getAsString(); //Get the game start time sent from the server
                        TimeInit = Integer.parseInt(TimeStarted); //Set the TimeStart to an int
                        if (CurrentWord.equals("ERROR") || CurrentWord.equals("DONE") || Status.equals("NOGAMEFOUND")){
                            running = false; //If game does not exist on server, end on client
                        }
                    } catch (Exception exception) {
                        System.out.println(exception);
                    }

                    wordView.setText(CurrentWord); //Set the text to whatever was received from the server as the word

                    if (live == false) {
                        countDownTimer.start(); //Reset countdown if it stops upon receiving server information
                    }

                }
            }
            else status.setText("Connection error"); // If you receive no information from the server or a null response, indicate a serve connection error
        }
    }
        public void exitClick(View view) {
            if (exiting) { //If you can exit a currently running game
                if (ID != null) { //If the game exits
                    if (!ID.equals("")) { //If the game exists and has been given a proper ID
                       while(score<winAmount+(winAmount-1) && !ID.equals("0")) {
                           RequestTask requestTask = new RequestTask();
                           requestTask.execute("GET /wongame/" + ID); //End the game on server side
                           score++;
                       }
                    }
                }
                Intent intent = new Intent(InGameActivity.this, MainActivity.class);
                startActivity(intent); //Send app to home page
            }
        }

        @RequiresApi(api = Build.VERSION_CODES.M)
        public void btnCameraClick(View view) {
        if (btnCamera.getText().equals("REPLAY")) { //If the button is for replay, repurpose the click
            oldID = ID; // Record last game ID
            score = 0; // Set the score to 0 for new game
            scoreShown.setText("0"); // Set the variable shown to 0
            status.setText(""); // Remove the status
            wordView.setText("Waiting..."); //Set the word to waiting while connecting to another
            running = true; // Set the game to a running mode and use the onTick to check server information
            btnCamera.setText("CAMERA"); //Repurpose button back to Camera
            RequestTask requestTask = new RequestTask();
            requestTask.execute("GET /startgame/"); //Ask Server for a new game
            return;
        }
        else if (timeRemain < 5) { //Add difficulty for Watson to send information to the incorrect game instance
            return;
        }
            if (checkSelfPermission(Manifest.permission.CAMERA) //Check permission to use camera
                    != PackageManager.PERMISSION_GRANTED) {

                requestPermissions(new String[]{Manifest.permission.CAMERA},
                        REQUEST_IMAGE_CAPTURE);
            }
            Intent takePictureIntent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE); //Request to use camera
            status.setText("Thinking....");
            // Ensure that there's a camera activity to handle the intent
            if (takePictureIntent.resolveActivity(getPackageManager()) != null) {
                // Create the File where the photo should go
                File photoFile = null;
                try {
                    photoFile = createImageFile();
                } catch (IOException ex) {
                    // Error occurred while creating the File
                    System.out.println(ex.getLocalizedMessage());
                }
                // Continue only if the File was successfully created
                if (photoFile != null) {
                    Uri photoURI = FileProvider.getUriForFile(this,
                            "com.example.android.fileprovider",
                            photoFile);
                    System.out.println(getContentResolver().getType(photoURI));
                    Cursor returnCursor =
                            getContentResolver().query(photoURI, null, null, null, null);
                    int nameIndex = returnCursor.getColumnIndex(OpenableColumns.DISPLAY_NAME);
                    int sizeIndex = returnCursor.getColumnIndex(OpenableColumns.SIZE);
                    returnCursor.moveToFirst();
                    System.out.println((returnCursor.getString(nameIndex)));
                    System.out.println((Long.toString(returnCursor.getLong(sizeIndex))));

                    takePictureIntent.putExtra(MediaStore.EXTRA_OUTPUT, photoURI);
                    startActivityForResult(takePictureIntent, REQUEST_IMAGE_CAPTURE); //Use photo to move forward with Watson
                }
            }
        }
  //  };
    private File createImageFile() throws IOException {
        // Create an image file name
        String timeStamp = new SimpleDateFormat("yyyyMMdd_HHmmss").format(new Date());
        String imageFileName = "JPEG_" + timeStamp + "_";
        File storageDir = getExternalFilesDir(Environment.DIRECTORY_PICTURES);
        File image = File.createTempFile(
                imageFileName,  /* prefix */
                ".jpg",         /* suffix */
                storageDir      /* directory */
        );

        // Save a file: path for use with ACTION_VIEW intents
        mCurrentPhotoPath = image.getAbsolutePath();
        return image;
    }


    //This happens after they take a picture.
    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {

        //if the picture exists, start watson AsyncTask.
        if (mCurrentPhotoPath != null) {
            RetrieveClassificationTask wattyTask = (RetrieveClassificationTask) new RetrieveClassificationTask().execute("Because I had to.");
        }
    }

    //this connects to IBM Watson and returns the value of what Watson believe the image is.
    class RetrieveClassificationTask extends AsyncTask<String, Void, ClassifiedImages> {

        private Exception exception;

        protected ClassifiedImages doInBackground(String...urls) {
            try {
                // Get the dimensions of the bitmap
                BitmapFactory.Options bmOptions = new BitmapFactory.Options();
                bmOptions.inJustDecodeBounds = true;

                // Determine how much to scale down the image to send to Watson with no issue (limited size request to Watson)
                int scaleFactor = 2;

                // Decode the image file into a Bitmap sized to fill the View
                bmOptions.inJustDecodeBounds = false;
                bmOptions.inSampleSize = scaleFactor;
                bmOptions.inPurgeable = true;

                Bitmap bitmap = BitmapFactory.decodeFile(mCurrentPhotoPath, bmOptions);

                //get rid of that garb because aint nobody got space for dat
                mCurrentPhotoPath = null;

                //compress it to yummy bites for watty
                ByteArrayOutputStream bos = new ByteArrayOutputStream();
                bitmap.compress(Bitmap.CompressFormat.JPEG, 0 /*ignored for PNG*/, bos);
                byte[] bitmapdata = bos.toByteArray();
                ByteArrayInputStream bs = new ByteArrayInputStream(bitmapdata);

                ClassifyOptions classifyOptions = new ClassifyOptions.Builder()
                        .imagesFile(bs)
                        .imagesFilename("scavengerhunt.png")
                        .build();

                //Send it to Watson and receive the result
                return watty.classify(classifyOptions).execute();
            } catch (Exception e) {
                this.exception = e;
                return null;
            }
        }

        protected void onPostExecute(ClassifiedImages result) {
            //Get the response of Watson
            boolean response = false; //Make a variable to check if Watson's response matches the provided server value
            List<String> results = new ArrayList<>();
            if (result.getImages() != null) { //Check that Watson did not give an empty list
                List<ClassifierResult> resultClassifiers = result.getImages().get(0).getClassifiers(); //Get classifiers from the list
                if (resultClassifiers == null ){ //If the classifiers are null, Watson error will be shown to user
                    status.setText("Try again!");
                }
                else if (resultClassifiers.size() > 0) { //If it is not null, check that it is not empty
                    List<ClassResult> resultClasses = resultClassifiers.get(0).getClasses(); //Get the classes from the classifiers
                    if (resultClasses.size() > 0) {  //If the classes are greater than 0 in size,  get all the names and compare them to the CurrentWord
                        for (int i = 0; i < resultClasses.size(); i++) {
                            results.add(resultClasses.get(i).getClassName());
                            System.out.println(resultClasses.get(i).getClassName());
                            //System.out.println(results.get(i));
                            if (CurrentWord.toLowerCase() == results.get(i).toLowerCase() || results.get(i).toLowerCase().contains(CurrentWord.toLowerCase()) || CurrentWord.toLowerCase().contains(results.get(i))) {
                                response = true; //Set the response to true if it matches the CurrentWord within certainty
                            }

                        }
                    }
                    if (response) { //Update if a round is won
                        status.setText("Success! New:");
                        score++;
                        scoreShown.setText(String.valueOf(score));
                        RequestTask requestTask = new RequestTask();
                        requestTask.execute("GET /wongame/" + ID);//Send round Won to server

                    } else { //Inform user that a round was not won
                        status.setText("Failure!");
                    }
                }
            }

        }
    }
}
