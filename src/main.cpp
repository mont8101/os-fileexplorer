#include <iostream>
#include <filesystem>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>


#define WIDTH 800
#define HEIGHT 600

typedef struct ImageData {
    SDL_Texture *folder;
    SDL_Texture *code;
    SDL_Texture *executable;
    SDL_Texture *other;
    SDL_Texture *image;
    SDL_Texture *video;
} ImageData;

typedef struct AppData {
    SDL_Texture *phrase;
    TTF_Font *font;
    TTF_Font *headerfont;
    SDL_Texture *tree;
    char* directories;
    ImageData *imagedata;
    
} AppData;



class file{

    public:
        std::string filename;
        int size;

}; //file

void initialize(SDL_Renderer *renderer, AppData *data_ptr, char* home);
void render(SDL_Renderer *renderer, AppData *data_ptr);
double formatSize(int size);
bool stringSortComparator(std::string s1, std::string s2) { 
    transform(s1.begin(), s1.end(), s1.begin(), tolower);
    transform(s2.begin(), s2.end(), s2.begin(), tolower);
    return s1 < s2; }

int main(int argc, char **argv)
{
    char *home = getenv("HOME");
    printf("HOME: %s\n", home);

    // initializing SDL as Video
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    // create window and renderer
    SDL_Renderer *renderer;
    SDL_Window *window;
    SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, 0, &window, &renderer);

    // initialize and perform rendering loop
    AppData data;
    data.directories = home;
    initialize(renderer, &data, home);
    render(renderer, &data);
    
    SDL_Event event;
    SDL_WaitEvent(&event);
    
    while (event.type != SDL_QUIT)
    {
        int needrender = 0;
        SDL_WaitEvent(&event);
        if(event.type == SDL_MOUSEBUTTONUP)
        {
            std::cout << event.type << std::endl;
        }
        if(needrender = 1){ render(renderer, &data); }
        
    }

    // clean up
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    TTF_Quit();


    return 0;
}

void initialize(SDL_Renderer *renderer, AppData *data_ptr, char* home)
{
    //font and font size
    
    data_ptr->font = TTF_OpenFont("resrc/OpenSans-Regular.ttf", 16);
    data_ptr->headerfont = TTF_OpenFont("resrc/OpenSans-Regular.ttf", 20);

    SDL_Surface *img_surf;
    img_surf = IMG_Load("resrc/other.jpg");
    data_ptr->imagedata->other = SDL_CreateTextureFromSurface(renderer, img_surf);
    img_surf = IMG_Load("resrc/video.jpg");
    data_ptr->imagedata->video = SDL_CreateTextureFromSurface(renderer, img_surf);
    img_surf = IMG_Load("resrc/folder.jpg");
    data_ptr->imagedata->folder = SDL_CreateTextureFromSurface(renderer, img_surf);
    img_surf = IMG_Load("resrc/executable.jpeg");
    data_ptr->imagedata->executable = SDL_CreateTextureFromSurface(renderer, img_surf);
    img_surf = IMG_Load("resrc/image.png");
    data_ptr->imagedata->image = SDL_CreateTextureFromSurface(renderer, img_surf);
    img_surf = IMG_Load("resrc/code.jpeg");
    data_ptr->imagedata->code = SDL_CreateTextureFromSurface(renderer, img_surf);
    SDL_FreeSurface(img_surf);

}

void render(SDL_Renderer *renderer, AppData *data_ptr)
{
    // set color of background when erasing frame
    SDL_SetRenderDrawColor(renderer, 22, 100, 37, 255);
    // erase renderer content
    SDL_RenderClear(renderer);

    //top rectangle (blue)
    SDL_Rect rect;
    rect.x = 25;
    rect.y = 15;
    rect.w = 750;
    rect.h = 30;

    SDL_SetRenderDrawColor(renderer, 30, 60, 150, 255);
    SDL_RenderFillRect(renderer, &rect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &rect);

    //left rectangle (gray)
    rect.x = 25;
    rect.y = 45;
    rect.w = 300;
    rect.h = 550;

    SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
    SDL_RenderFillRect(renderer, &rect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &rect);
    
    // header text handling



    //directory text handling
    SDL_Rect thin_rect;
    thin_rect.x = 30;
    thin_rect.y = 70;
    thin_rect.w = 250;
    thin_rect.h = 1;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    int texty = 50;
    rect.y = texty;
    SDL_Color color = { 0, 0, 0};
    std::vector<std::string> text_list;
    for(const auto & entry : std::filesystem::directory_iterator(data_ptr->directories))
    {
        std::string clean_entry = entry.path().string();
        clean_entry = clean_entry.substr(clean_entry.find_last_of("/")+1);
        if(!(clean_entry.substr(0,1) == "." && clean_entry.substr(0, 2) != ".."))
        {
            text_list.push_back(clean_entry);
        }
    }
    std::sort(text_list.begin(), text_list.end(), stringSortComparator); 
    for(int i = 0; i < text_list.size(); i++)
    {
        const char* entry = text_list.at(i).c_str();
        SDL_Surface *phrase_surf = TTF_RenderText_Solid(data_ptr->font, entry, color);
        data_ptr->phrase = SDL_CreateTextureFromSurface(renderer, phrase_surf);
        SDL_FreeSurface(phrase_surf);

        SDL_QueryTexture(data_ptr->phrase, NULL, NULL, &(rect.w), &(rect.h));
        rect.y = texty;
        rect.x = 30;
        SDL_RenderCopy(renderer, data_ptr->phrase, NULL, &rect);
        texty = texty + 18;

        SDL_RenderFillRect(renderer, &thin_rect);
        thin_rect.y = thin_rect.y + 18;
    }

    
    //gets size of texture, store it in rect
    //
    //rect.x = 30;
    //rect.y = 30;
    //

    // show rendered frame
    SDL_RenderPresent(renderer);
}



//use as input the size in kilobytes and convert to the easiest readable size
double formatSize(int size)
{
    return 0;
}//formatSize

//std::vector<std::string> getFilePaths(std::string path)
//{
//   return NULL;
//}