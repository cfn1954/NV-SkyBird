/* Uncomment to compile from TextWrangler
#!/bin/sh
/usr/bin/gcc -xc -o temp -lopenal - <<ENDOFCODE
*/

#include <stdio.h>
#include <stdlib.h>
// OS X specific headers
#include <AL/al.h>
#include <AL/alc.h>


int main(int argc, char *argv[])
{  
	char* myFilename = argv[1]; 
	
	if( argc == 2 )
   {
      printf("The argument supplied is %s\n", argv[1]);
   }
   else if( argc > 2 )
   {
      printf("Too many arguments supplied.\n");
   }
   else
   {
      printf("One argument expected.\n");
      myFilename = "Sound1.wav"; 
   }


	FILE *fp = NULL;
	fp=fopen(myFilename,"rb");
	if (!fp) 
	{
		printf("Error Opening File\n");
		exit(0);
	}

	//Remember that integer sizes vary by compiler and system
	//Variables to store info about the WAVE file (all of them are not needed for OpenAL)
	char type[4];
	unsigned int size,chunkSize;
	short formatType,channels;
	unsigned int sampleRate,avgBytesPerSec;
	short bytesPerSample,bitsPerSample;
	unsigned int dataSize;

	//Check that the WAVE file is OK, and pull other parts of header


	fread(type,sizeof(char),4,fp);														//read Header RIFF
	if(type[0]!='R' || type[1]!='I' || type[2]!='F' || type[3]!='F')
	{
		printf("Expected RIFF tag\n");
		exit(2);
	}
	printf("Found RIFF tag\n");

	fread(&size, sizeof(unsigned int),1,fp);								//Needed later, read Header
	fread(type, sizeof(char),4,fp);												//read Header WAVE
	if (type[0]!='W' || type[1]!='A' || type[2]!='V' || type[3]!='E')
	{
    	printf("Expected WAVE tag\n");
    	exit(3);
    }
    printf("Found WAVE tag\n");
    
	fread(type,sizeof(char),4,fp);													//read Header  fmt
	if (type[0]!='f' || type[1]!='m' || type[2]!='t' || type[3]!=' ')
	{
		printf("Expected fmt tag\n");
		exit(4);
	}
	printf("Found fmt tag\n");
	
	fread(&chunkSize,sizeof(unsigned int),1,fp);
	fread(&formatType,sizeof(short),1,fp);
	fread(&channels,sizeof(short),1,fp);
	fread(&sampleRate,sizeof(unsigned int),1,fp);
	fread(&avgBytesPerSec,sizeof(unsigned int),1,fp);
	fread(&bytesPerSample,sizeof(short),1,fp);
	fread(&bitsPerSample,sizeof(short),1,fp);

	fread(type,sizeof(char),4,fp);													//read Header  data tag
	if (type[0]!='d' || type[1]!='a' || type[2]!='t' || type[3]!='a')
	{
		printf("Expected data tag \n");											//an error here usually mean an incompatible wav file format
	   exit(5);
	}
   
	printf("Found data tag \n");
	// If we got here header is OK
	printf("File Format ok \n");

	fread(&dataSize,sizeof(unsigned int),1,fp);

	//  This is where we store PCM data, this and the following two lines is where others went wrong
	unsigned char* buf[dataSize];  
	//fread(buf,sizeof(unsigned char),dataSize,fp) ;
	//fclose(fp);

   //Display the info about the WAVE file
	printf("Chunk Size:  %u \n",  chunkSize);
	printf("Format Type: %u \n", formatType);
	printf("Channels: %u \n", channels);
	printf("Sample Rate: %u \n", sampleRate);
	printf("Average Bytes Per Second: %u \n", avgBytesPerSec);
	printf("Bytes Per Sample: %u \n", bytesPerSample);
	printf("Bits Per Sample: %u \n", bitsPerSample);
	printf("Data Size: %u \n", dataSize);
	printf("Bytes Loaded: %lu \n\n\n", fread(buf,sizeof(unsigned char),dataSize,fp));			//  cout << fread(buf,sizeof(BYTE),dataSize,fp) << " bytes loaded\n";  
	fclose(fp); 

	ALCdevice *device;
	ALCcontext *context;
	device = alcOpenDevice(NULL);
	context = alcCreateContext(device, NULL);
	alcMakeContextCurrent(context);
	
	ALuint source;
	ALuint buffer;
	ALuint frequency=sampleRate;
	ALenum format=0;

	alGenBuffers(1, &buffer);
	alGenSources(1, &source);

	if(bitsPerSample == 8)
		{
	    if(channels == 1)
	        format = AL_FORMAT_MONO8;
	    else if(channels == 2)
        		format = AL_FORMAT_STEREO8;
		}
	else if(bitsPerSample == 16)
		{
    	if(channels == 1)
        	format = AL_FORMAT_MONO16;
    		else if(channels == 2)
        		format = AL_FORMAT_STEREO16;
		}

	alBufferData(buffer, format, buf, dataSize, frequency);

	//Sound setting variables
	ALfloat SourcePos[] = { 0.0, 0.0, 0.0 };                                  //Position of the source sound
	ALfloat SourceVel[] = { 0.0, 0.0, 0.0 };                                   //Velocity of the source sound
	ALfloat ListenerPos[] = { 0.0, 0.0, 0.0 };                                //Position of the listener
	ALfloat ListenerVel[] = { 0.0, 0.0, 0.0 };									//Velocity of the listener
	ALfloat ListenerOri[] = { 0.0, 0.0, -1.0,  0.0, 1.0, 0.0 }; 		//Orientation of the listener

	alListenerfv(AL_POSITION,    ListenerPos);                          //Set position of the listener
	alListenerfv(AL_VELOCITY,    ListenerVel);                           //Set velocity of the listener
	alListenerfv(AL_ORIENTATION, ListenerOri);                      //Set orientation of the listener

	alSourcei (source, AL_BUFFER,   buffer);                             //Link the buffer to the source
	alSourcef (source, AL_PITCH,    1.0f     );                              //Set the pitch of the source
	alSourcef (source, AL_GAIN,     1.0f     );                              //Set the gain of the source
	alSourcefv(source, AL_POSITION, SourcePos);                  //Set the position of the source
	alSourcefv(source, AL_VELOCITY, SourceVel);                   //Set the velocity of the source
	alSourcei (source, AL_LOOPING,  AL_FALSE );                   //Set if source is looping sound

	//PLAY 
	alSourcePlay(source);																//Play the sound buffer linked to the source
	printf("Press Enter to Close\n");
	system("read");                                                            			//Pause to let the sound play

	//Clean-up

	alDeleteSources(1, &source);                                                //Delete the OpenAL Source
	alDeleteBuffers(1, &buffer);                                                 	//Delete the OpenAL Buffer
	alcMakeContextCurrent(NULL);                                           //Make no context current
	alcDestroyContext(context);                                                 //Destroy the OpenAL Context
	alcCloseDevice(device);                                                         //Close the OpenAL Device

	return EXIT_SUCCESS;                                                        
}

/* Uncomment to compile from TextWrangler
ENDOFCODE
./temp
read
*/
