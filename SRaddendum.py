import speech_recognition as sr
from datetime import datetime
import requests

# Function to respond with the current time
def respond_with_time():
    current_time = datetime.now().strftime("%I:%M %p")
    print("Current time is:", current_time)

# Function to display a happy emoji on the LED display
def display_happy_emoji():
    url = 'http://192.168.53.215'  # Replace with the correct URL of your LED display server
    payload = {'displaydata': 'happy'}  # Replace with the code or data to display the happy emoji
    response = requests.get(url, params=payload)
    print(response.text)
    return response.text

# Initialize the recognizer
r = sr.Recognizer()

while True:
    # Use the default microphone as the audio source
    with sr.Microphone() as source:
        print("Say something!")
        # Adjust for ambient noise
        r.adjust_for_ambient_noise(source)
        # Record audio from microphone
        audio = r.listen(source)

    # Convert speech to text
    try:
        text = r.recognize_google(audio)
        print("Got it! Now to recognize it.")
        print(f"You said: {text}")

        if text == "time":
            text = datetime.now().strftime("%I:%M %p")

        if text == "date":
            text = datetime.now().strftime("%A %d %B %Y")

        if text.lower() == "secret":
            text = "This project was created by the embedded interns"

        if text.lower() == "happy":
            display_happy_emoji()
            continue  # Skip the request to the LED display server

        url = 'http://192.168.24.215'
        payload = {'displaydata': text}
        response = requests.get(url, params=payload)

        print(response.text)

    except sr.UnknownValueError:
        print("Speech recognition could not understand audio")
    except sr.RequestError as e:
        print(f"Could not request results from Google Speech Recognition service; {e}")
