/*
Compiled via command line using:
	g++ ballHell.cpp -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -o ballHell
*/

//Using SDL, SDL_image, standard IO, vectors, and strings
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <cmath>

#define PI 3.14159265

//Screen dimension constants
const int SCREEN_WIDTH = 1200;
const int SCREEN_HEIGHT = 600;

//A circle structure
struct Circle{
	//Dimensions
	double x, y;
	double r;
	
	//Magnitude and direction of the velocity
	double m;
	double ux, uy;
};

//A collision structure
struct Collision{
	//Magnitude
	float m;
	
	//Radius x 2
	float r;
	
	//Normal and tangent vectors
	float nx, ny;
	float tx, ty;
	
	//Projection along n and t
	float n1, n2;
	float t1, t2;
};

//Texture wrapper class
class LTexture{
	public:
		//Initialize variables
		LTexture();
		
		//Deallocates memory
		~LTexture();
		
		//Loads image at specified path
		bool loadFromFile(std::string);
		
		#ifdef _SDL_TTF_H
		//Creates image from font string
		bool loadFromRenderedText(std::string, SDL_Color, TTF_Font*);
		#endif
		
		//Deallocates texture
		void free();
		
		//Set color modulation
		void setColor(Uint8, Uint8, Uint8);
		
		//Set blending
		void setBlendMode(SDL_BlendMode);
		
		//Set alpha modulation
		void setAlpha(Uint8 alpha);
		
		//Renders texture at given point
		void render(int, int, SDL_Rect* = NULL, double = 0.0, SDL_Point* = NULL, SDL_RendererFlip = SDL_FLIP_NONE);
		
		//Gets image dimensions
		int getWidth();
		int getHeight();
		
	private:
		//The actual hardware texture
		SDL_Texture* mTexture;
		
		//Image dimensions
		int mWidth;
		int mHeight;
};

//Timer wrapper class
class LTimer{
	public:
		//Initializes variables
		LTimer();
		
		//The various clock actions
		void start();
		void stop();
		void pause();
		void unpause();
		
		//Gets the timer's time
		Uint32 getTicks();
		
		//Checks the status of the timer
		bool isStarted();
		bool isPaused();
		
	private:
		//The clock time when the timer started
		Uint32 mStartTicks;
		
		//The ticks stored when the timer was paused
		Uint32 mPausedTicks;
		
		//The timer status
		bool mPaused;
		bool mStarted;
};

//The balls to be bounced
class Ball{
	public:
		//The dimensions of the ball
		static const int BALL_WIDTH = 20;
		static const int BALL_HEIGHT = 20;
		
		//Maximum axis velocity of the ball
		static const int MAX_VEL = 5;
		
		//Initializes the variables
		Ball(double, double, double, double);
		
		//Moves the ball
		void move(int);
		
		//Shows the ball on the screen
		void render();
		
		//Shift the ball
		void shift(double, double);
		
		//The ball's collision circle
		Circle mBall;
	
	private:
		//Pointer to ball texture image
		LTexture* mBallTexture;
};

//Starts up SDL and creates window
bool init();

//Loads media
bool loadMedia();

//Frees media and shuts down SDL
void close();

//Checks if two specific balls are colliding
bool checkCollision(Circle&, Circle&);

//Swap two variables
void swap(double*, double*);

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//The ball's textures
LTexture gBallTexture;

//The global collision structure
Collision gCollision;

//Vectors for balls and enemies
std::vector<Ball> gBalls;

int main(int argc, char *args[]){	
	//Start up SDL and create window
	if(!init()){
		printf("Failed to initialize!\n");
	}else{
		//Load media
		if(!loadMedia()){
			printf("Failed to load media!\n");
		}else{
			//Main loop flag
			bool quit = false;
			
			//Event handler
			SDL_Event e;
			
			//Number of balls spawned
			const int BALLS = 50;
			
			for(int i = 0; i < BALLS; ++i){
				Ball ball(
					rand()%(SCREEN_WIDTH-Ball::BALL_WIDTH),
					rand()%(SCREEN_HEIGHT-Ball::BALL_HEIGHT),
					(rand()%5+1)*cos(i), (rand()%5+1)*sin(i));
				gBalls.push_back(ball);
			}
			
			//While application is running
			while(!quit){									
				//Handle events on queue
				while(SDL_PollEvent(&e) != 0){
					//User requests quit
					if(e.type == SDL_QUIT){
						quit = true;
					}
				}
				
				//Clear screen
				SDL_SetRenderDrawColor(gRenderer, 0xB4, 0xB4, 0xB4, 0xFF);
				SDL_RenderClear(gRenderer);
				
				//Move and render balls
				for(int i = 0; i < gBalls.size(); ++i){
					gBalls[i].move(i);
					gBalls[i].render();
				}
				
				//Update screen
				SDL_RenderPresent(gRenderer);
			}
		}
	}
	
	//Free resources and close SDL
	close();
	
	return 0;
}

LTexture::LTexture(){
	//Initialize
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

LTexture::~LTexture(){
	//Deallocate
	free();
}

bool LTexture::loadFromFile(std::string path){
	//Get rid of preexisting texture
	free();
	
	//The final texture
	SDL_Texture* newTexture = NULL;
	
	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	
	if(loadedSurface == NULL){
		printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
	}else{
		//Color key image
		SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0xFF, 0xFF, 0xFF));
		
		//Create texture from surface pixels
		newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
		
		if(newTexture == NULL){
			printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		}else{
			//Get image dimensions
			mWidth = loadedSurface->w;
			mHeight = loadedSurface->h;
		}
		
		//Get rid of loaded surface
		SDL_FreeSurface(loadedSurface);
	}
	
	//Return success
	mTexture = newTexture;
	return mTexture != NULL;
}

#ifdef _SDL_TTF_H
bool LTexture::loadFromRenderedText(std::string textureText, SDL_Color textColor, TTF_Font* font){
	//Get rid of preexisting texture
	free();
	
	//Render text surface
	SDL_Surface* textSurface = TTF_RenderText_Solid(font, textureText.c_str(), textColor);
	if(textSurface == NULL){
		printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
	}else{
		//Create texture from surface pixels
		mTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
		if(mTexture == NULL){
			printf("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
		}else{
			//Get image dimensions
			mWidth = textSurface->w;
			mHeight = textSurface->h;
		}
		
		//Get rid of old surface
		SDL_FreeSurface(textSurface);
	}
	
	//Return success
	return mTexture != NULL;
}
#endif

void LTexture::free(){
	//Free texture if it exists
	if(mTexture != NULL){
		SDL_DestroyTexture(mTexture);
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

void LTexture::setColor(Uint8 red, Uint8 green, Uint8 blue){
	//Modulate texture
	SDL_SetTextureColorMod(mTexture, red, green, blue);
}

void LTexture::setBlendMode(SDL_BlendMode blending){
	//Set blending function
	SDL_SetTextureBlendMode(mTexture, blending);
}

void LTexture::setAlpha(Uint8 alpha){
	//Modulate texture alpha
	SDL_SetTextureAlphaMod(mTexture, alpha);
}

void LTexture::render(int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip){
	//Set rendering space and render to screen
	SDL_Rect renderQuad = {x, y, mWidth, mHeight};
	
	//Set clip rendering dimensions
	if(clip != NULL){
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}
	
	//Render to screen
	SDL_RenderCopyEx(gRenderer, mTexture, clip, &renderQuad, angle, center, flip);
}	

int LTexture::getWidth(){
	return mWidth;
}

int LTexture::getHeight(){
	return mHeight;
}

LTimer::LTimer(){
	//Initialize the variables
	mStartTicks = 0;
	mPausedTicks = 0;
	
	mPaused = false;
	mStarted = false;
}

void LTimer::start(){
	//Start the timer
	mStarted = true;
	
	//Unpause the timer
	mPaused = false;
	
	//Get the current clock time
	mStartTicks = SDL_GetTicks();
	mPausedTicks = 0;
}

void LTimer::stop(){
	//Stop the timer
	mStarted = false;
	
	//Unpause the timer
	mPaused = false;
	
	//Clear tick variables
	mStartTicks = 0;
	mPausedTicks = 0;
}

void LTimer::pause(){
	//If the timer is running and isn't already paused
	if(mStarted && !mPaused){
		//Pause the timer
		mPaused = true;
		
		//Calculate the paused ticks
		mPausedTicks = SDL_GetTicks()-mStartTicks;
		mStartTicks = 0;
	}
}

void LTimer::unpause(){
	//If the timer is running and paused
	if(mStarted && mPaused){
		//Unpause the timer
		mPaused = false;
		
		//Reset the startng ticks
		mStartTicks = SDL_GetTicks()-mPausedTicks;
		
		//Reset the paused ticks
		mPausedTicks = 0;
	}
}

Uint32 LTimer::getTicks(){
	//The actual timer time
	Uint32 time = 0;
	
	//If the timer is running
	if(mStarted){
		//If the timer is paused
		if(mPaused){
			//Return the number of ticks when the timer is paused
			time = mPausedTicks;
		}else{
			//Return the current time minus the start time
			time = SDL_GetTicks()-mStartTicks;
		}
	}
	
	return time;
}

bool LTimer::isStarted(){
	//Timer is running and paused or unpaused
	return mStarted;
}

bool LTimer::isPaused(){
	//Timer is running and paused
	return mPaused && mStarted;
}

Ball::Ball(double sPosX, double sPosY, double sVelX, double sVelY){	
	//Initializes the ball pointer
	mBallTexture = &gBallTexture;
	
	//Initialize the dimensions
	mBall.x = sPosX;
	mBall.y = sPosY;
	mBall.r = BALL_WIDTH/2;
	
	//Initializes the unit vector and magnitude
	mBall.m = sqrt(pow(sVelX, 2)+pow(sVelY, 2));
	mBall.ux = sVelX/mBall.m;
	mBall.uy = sVelY/mBall.m;
}

void Ball::move(int index){
	//Collision
	for(int i = 0; i < gBalls.size(); ++i){
		if(i != index && checkCollision(mBall, gBalls[i].mBall)){
			mBall.ux = gCollision.tx*gCollision.t1+gCollision.nx*gCollision.n2;
			mBall.uy = gCollision.ty*gCollision.t1+gCollision.ny*gCollision.n2;
			
			gBalls[i].mBall.ux = gCollision.tx*gCollision.t2+gCollision.nx*gCollision.n1;
			gBalls[i].mBall.uy = gCollision.ty*gCollision.t2+gCollision.ny*gCollision.n1;
			
			swap(&mBall.m, &gBalls[i].mBall.m);
			
			shift(-(gCollision.r-gCollision.m)*gCollision.nx/2, -(gCollision.r-gCollision.m)*gCollision.ny/2);
			gBalls[i].shift((gCollision.r-gCollision.m)*gCollision.nx/2, (gCollision.r-gCollision.m)*gCollision.ny/2);
		}
	}
	
	//If the ball went too far left or right
	if(mBall.x < 0){
		mBall.x = 0;
		mBall.ux *= -1;
	}else if(mBall.x+BALL_WIDTH > SCREEN_WIDTH){
		mBall.x = SCREEN_WIDTH-BALL_WIDTH;
		mBall.ux *= -1;
	}
	
	//If the ball went too far up or down
	if(mBall.y < 0){
		mBall.y = 0;
		mBall.uy *= -1;
	}else if(mBall.y+BALL_HEIGHT > SCREEN_HEIGHT){
		mBall.y = SCREEN_HEIGHT-BALL_HEIGHT;
		mBall.uy *= -1;
	}
	
	//Move the ball
	shift(mBall.ux*mBall.m, mBall.uy*mBall.m);
}

void Ball::shift(double x, double y){
	mBall.x += x;
	mBall.y += y;
}

void Ball::render(){
	//Show the ball
	mBallTexture->render(mBall.x, mBall.y);
}

bool init(){
	//Initialization flag
	bool success = true;
	
	//Initialize SDL
	if(SDL_Init(SDL_INIT_VIDEO) < 0){
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		success = false;
	}else{
		//Set texture filtering to linear
		if(!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")){
			printf("Warning: Linear texture filtering not enabled!");
		}
		
		//Create window
		gWindow = SDL_CreateWindow("Bouncing Ball", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if(gWindow == NULL){
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}else{
			//Create renderer for window
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			
			if(gRenderer == NULL){
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}else{
				//Initialize renderer color
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
				
				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if(!(IMG_Init(imgFlags) & imgFlags)){
					printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
					success = false;
				}
			}
		}
	}
	
	return success;
}

bool loadMedia(){
	//Loading success flag
	bool success = true;
	
	//Load ball texture
	if(!gBallTexture.loadFromFile("Resources/ball.bmp")){
		printf("Failed to load ball texture!\n");
		success = false;
	}
	
	return success;
}

void close(){
	//Free loaded images
	gBallTexture.free();
	
	//Destroy window
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;
	
	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

bool checkCollision(Circle& c1, Circle& c2){
	//Computes for the magnitude
	gCollision.m = sqrt(pow(c2.x-c1.x, 2)+pow(c2.y-c1.y, 2));
	
	//Computes for the radius sum
	gCollision.r = c1.r+c2.r;
	
	if(gCollision.m < gCollision.r){
		//Computes for the normal vector
		gCollision.nx = (c2.x-c1.x)/gCollision.m;
		gCollision.ny = (c2.y-c1.y)/gCollision.m;
		
		//Computes for the tangent vector
		gCollision.tx = -(c2.y-c1.y)/gCollision.m;
		gCollision.ty = (c2.x-c1.x)/gCollision.m;
		
		//Computes for the projection along the normal
		gCollision.n1 = c1.ux*gCollision.nx+c1.uy*gCollision.ny;
		gCollision.n2 = c2.ux*gCollision.nx+c2.uy*gCollision.ny;
		
		//Computes for the projection along the tangent
		gCollision.t1 = c1.ux*gCollision.tx+c1.uy*gCollision.ty;
		gCollision.t2 = c2.ux*gCollision.tx+c2.uy*gCollision.ty;
		
		return true;
	}
	
	return false;
}

void swap(double* a, double* b){
	double t = *a;
	*a = *b;
	*b = t;
}