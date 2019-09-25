package ca.uwo.eng.se3313.project.scavenggerhunt;

import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;

public class MainActivity extends AppCompatActivity {
    //Creating a home page and a page to exit the android app to in order to not connect to a game.

    public MainActivity() {

    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

    }
    public void startGame(View view){
        Intent intent = new Intent(MainActivity.this, InGameActivity.class);
        startActivity(intent);
    }

    //Initialize Visual Recognition client
    VisualRecognition service=new VisualRecognition(
            VisualRecognition.VERSION_DATE_2016_05_20,
            getString(R.string.api_key)

    );









}
