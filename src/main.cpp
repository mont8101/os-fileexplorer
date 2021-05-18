#include <iostream>
#include <filesystem>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>


#define WIDTH 800
#define HEIGHT 600



class File{

    public:
        std::string filename;
        std::string path;
        int size;
        SDL_Texture *texture_name;
        SDL_Texture *texture_extension;
        SDL_Texture *texture_size;
        SDL_Texture *texture_icon;
        SDL_Texture *texture_permissions;
        std::string extension;
        std::string size_suffix;
        std::string permissions;
        int x;
        int y;
        int w;
        int h;


}; //file

class Directory : public File{

    public: 
        std::vector<File*> contents;

};

typedef struct AppData {
    TTF_Font *font;
    TTF_Font *headerfont;
    SDL_Texture *header;
    SDL_Texture *recursive;
    std::vector<File*> files;
    SDL_Texture *folder;
    SDL_Texture *code;
    SDL_Texture *executable;
    SDL_Texture *other;
    SDL_Texture *image;
    SDL_Texture *video;
    int top;
    bool recursive_flag;
    int starting_y;
    std::string curr_folder_path;
} AppData;

void initialize(SDL_Renderer *renderer, AppData *data_ptr, char* home);
void render(SDL_Renderer *renderer, AppData *data_ptr);
int recursiveRender(std::string path, int starting_y, int starting_x, SDL_Renderer *renderer, AppData *data_ptr);
double formatSize(int size);
std::vector<File*> createFile(std::string path, AppData *data_ptr, SDL_Renderer *renderer);
void EventHandler(SDL_Event *event, AppData *data_ptr, SDL_Renderer *renderer);
bool isExecutable(std::string path);
std::string FilePermissions(std::string path);
bool FileSortComparator(File *s1, File *s2) { 
    transform(s1->filename.begin(), s1->filename.end(), s1->filename.begin(), tolower);
    transform(s2->filename.begin(), s2->filename.end(), s2->filename.begin(), tolower);
    return s1->filename < s2->filename; }

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
            EventHandler(&event, &data, renderer);
        }
        
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
    
    data_ptr->font = TTF_OpenFont("resrc/OpenSans-Regular.ttf", 24);
    data_ptr->headerfont = TTF_OpenFont("resrc/OpenSans-Regular.ttf", 30);
    SDL_Surface *img_surf;
    img_surf = IMG_Load("resrc/other.jpg");
    data_ptr->other = SDL_CreateTextureFromSurface(renderer, img_surf);
    img_surf = IMG_Load("resrc/video.jpg");
    data_ptr->video = SDL_CreateTextureFromSurface(renderer, img_surf);
    img_surf = IMG_Load("resrc/folder.jpg");
    data_ptr->folder = SDL_CreateTextureFromSurface(renderer, img_surf);
    img_surf = IMG_Load("resrc/executable.jpeg");
    data_ptr->executable = SDL_CreateTextureFromSurface(renderer, img_surf);
    img_surf = IMG_Load("resrc/image.png");
    data_ptr->image = SDL_CreateTextureFromSurface(renderer, img_surf);
    img_surf = IMG_Load("resrc/code.jpeg");
    data_ptr->code = SDL_CreateTextureFromSurface(renderer, img_surf);
    SDL_FreeSurface(img_surf);

    //header text
    SDL_Color color = { 255, 255, 255 };
    SDL_Surface *surface;
    std::string phrase = "     Filename                              Size              Permissions ";
    surface = TTF_RenderText_Solid(data_ptr->headerfont, phrase.c_str(), color);
    data_ptr->header = SDL_CreateTextureFromSurface(renderer, surface);
    
    //recursive text
    phrase = "Recursive";
    surface = TTF_RenderText_Solid(data_ptr->headerfont, phrase.c_str(), color);
    data_ptr->recursive = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    //intial conditions
    data_ptr->top = 0;
    data_ptr->recursive_flag = false;
    data_ptr->starting_y = 60;
    data_ptr->curr_folder_path = home;

    //loop through all directories and make files for everything inside
    //may only want to do this if directory is clicked
    data_ptr->files = createFile(home, data_ptr, renderer);
    

}

void render(SDL_Renderer *renderer, AppData *data_ptr)
{
    // set color of background when erasing frame
    SDL_SetRenderDrawColor(renderer, 22, 100, 37, 255);
    // erase renderer content
    SDL_RenderClear(renderer);
    SDL_Rect rect;

    //left rectangle (gray)
    rect.x = 25;
    rect.y = 55;
    rect.w = 750;
    rect.h = 550;

    SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
    SDL_RenderFillRect(renderer, &rect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &rect);

    //top rectangle (blue)
    rect.x = 25;
    rect.y = 15;
    rect.w = 750;
    rect.h = 40;

    SDL_SetRenderDrawColor(renderer, 30, 60, 150, 255);
    SDL_RenderFillRect(renderer, &rect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &rect);

    // header text handling
    SDL_QueryTexture(data_ptr->header, NULL, NULL, &(rect.w), &(rect.h));
    SDL_RenderCopy(renderer, data_ptr->header, NULL, &rect);

    //arrow down (gray)
    rect.x = 2;
    rect.y = 550;
    rect.w = 21;
    rect.h = 40;

    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderFillRect(renderer, &rect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &rect);

    if(data_ptr->top != 0)
    {
        //arrow up (gray)
        rect.x = 2;
        rect.y = 55;
        rect.w = 21;
        rect.h = 40;

        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        SDL_RenderFillRect(renderer, &rect);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &rect);
    }
    
    //recursive
    rect.x = 250;
    rect.y = 20;
    rect.w = 145;
    rect.h = 33;
    if(data_ptr->recursive_flag == true)
    {
        SDL_SetRenderDrawColor(renderer, 10, 160, 20, 255);
        SDL_RenderFillRect(renderer, &rect);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &rect);
    }
    else
    {
        SDL_SetRenderDrawColor(renderer, 160, 30, 30, 255);
        SDL_RenderFillRect(renderer, &rect);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &rect);
    }
    rect.x = 255;
    rect.y = 14;
    SDL_QueryTexture(data_ptr->recursive, NULL, NULL, &(rect.w), &(rect.h));
    SDL_RenderCopy(renderer, data_ptr->recursive, NULL, &rect);

    //thin rect
    SDL_Rect thin_rect;
    thin_rect.x = 60;
    thin_rect.y = data_ptr->starting_y+28;
    thin_rect.w = 300;
    thin_rect.h = 1;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    //text rect
    SDL_Rect text_rect;
    text_rect.x = 60;
    text_rect.y = data_ptr->starting_y;
    //size rect
    SDL_Rect size_rect;
    size_rect.x = 430;
    size_rect.y = data_ptr->starting_y;
    //permissions rect
    SDL_Rect permissions_rect;
    permissions_rect.x = 600;
    permissions_rect.y = data_ptr->starting_y;

    //image rect
    SDL_Rect image_rect;
    image_rect.x = 30;
    image_rect.y = data_ptr->starting_y;
    image_rect.w = 28;
    image_rect.h = 28;

    //variables
    std::vector<File*> file_list = data_ptr->files;
    int y_pos = data_ptr->starting_y;

    //rendering loop
    for(int i = data_ptr->top; i < file_list.size(); i++)
    {   

        //entry
        File* entry = file_list.at(i);

        //get size of texture
        SDL_QueryTexture(entry->texture_name, NULL, NULL, &(text_rect.w), &(text_rect.h));

        //render filename and icon
        SDL_RenderCopy(renderer, entry->texture_name, NULL, &text_rect);
        SDL_RenderCopy(renderer, entry->texture_icon, NULL, &image_rect);

        //set entry variables
        entry->x = text_rect.x;
        entry->y = text_rect.y;
        entry->w = text_rect.w + image_rect.w + 12;
        entry->h = text_rect.h;

        //render line
        SDL_RenderFillRect(renderer, &thin_rect);

        //get size of size texture
        SDL_QueryTexture(entry->texture_size, NULL, NULL, &(size_rect.w), &(size_rect.h));

        //render size
        SDL_RenderCopy(renderer, entry->texture_size, NULL, &size_rect);

        //render line
        thin_rect.x = 430;
        thin_rect.w = 160;
        SDL_RenderFillRect(renderer, &thin_rect);

        //permissions
        //get size of permissions texture
        SDL_QueryTexture(entry->texture_permissions, NULL, NULL, &(permissions_rect.w), &(permissions_rect.h));

        //update x location for thin line
        thin_rect.x = 600;

        //render permissions
        SDL_RenderCopy(renderer, entry->texture_permissions, NULL, &permissions_rect);

        //render line
        thin_rect.x = 600;
        thin_rect.w = 150;
        SDL_RenderFillRect(renderer, &thin_rect);

        //update y_pos in case of recursion
        y_pos = y_pos + 30;
        if(entry->extension == "Folder" && data_ptr->recursive_flag == true)
        {
            y_pos = recursiveRender(entry->path, y_pos, 75, renderer, data_ptr);
        }

        //update variables
        thin_rect.y = y_pos + 28;
        thin_rect.x = 60;
        thin_rect.w = 300;
        image_rect.y = y_pos;
        text_rect.y = y_pos;
        image_rect.y = y_pos;
        size_rect.y = y_pos;
        permissions_rect.y = y_pos;
    }

    // show rendered frame
    SDL_RenderPresent(renderer);
}

int recursiveRender(std::string path, int starting_y, int starting_x, SDL_Renderer *renderer, AppData *data_ptr)
{
    //thin rect
    SDL_Rect thin_rect;
    thin_rect.w = 360-starting_x;
    thin_rect.y = starting_y+28;
    thin_rect.w = 300;
    thin_rect.h = 1;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    //text rect
    SDL_Rect text_rect;
    text_rect.x = starting_x;
    text_rect.y = starting_y;
    //size rect
    SDL_Rect size_rect;
    size_rect.x = 430;
    size_rect.y = starting_y;
    //permissions rect
    SDL_Rect permissions_rect;
    permissions_rect.x = 600;
    permissions_rect.y = starting_y;

    //image rect
    SDL_Rect image_rect;
    image_rect.x = starting_x-30;
    image_rect.y = starting_y;
    image_rect.w = 28;
    image_rect.h = 28;

    //variables
    std::vector<File*> file_list = createFile(path, data_ptr, renderer);
    int y_pos = starting_y;

    //rendering loop
    for(int i = data_ptr->top; i < file_list.size(); i++)
    {   

        //entry
        File* entry = file_list.at(i);

        //get size of texture
        SDL_QueryTexture(entry->texture_name, NULL, NULL, &(text_rect.w), &(text_rect.h));

        //render filename and icon
        SDL_RenderCopy(renderer, entry->texture_name, NULL, &text_rect);
        SDL_RenderCopy(renderer, entry->texture_icon, NULL, &image_rect);

        //set entry variables
        entry->x = text_rect.x;
        entry->y = text_rect.y;
        entry->w = text_rect.w + image_rect.w + 12;
        entry->h = text_rect.h;

        //render line
        SDL_RenderFillRect(renderer, &thin_rect);

        //get size of size texture
        SDL_QueryTexture(entry->texture_size, NULL, NULL, &(size_rect.w), &(size_rect.h));

        //render size
        SDL_RenderCopy(renderer, entry->texture_size, NULL, &size_rect);

        //render line
        thin_rect.x = 430;
        thin_rect.w = 160;
        SDL_RenderFillRect(renderer, &thin_rect);

        //permissions
        //get size of permissions texture
        SDL_QueryTexture(entry->texture_permissions, NULL, NULL, &(permissions_rect.w), &(permissions_rect.h));

        //update x location for thin line
        thin_rect.x = 600;

        //render permissions
        SDL_RenderCopy(renderer, entry->texture_permissions, NULL, &permissions_rect);

        //render line
        thin_rect.x = 600;
        thin_rect.w = 150;
        SDL_RenderFillRect(renderer, &thin_rect);

        //update y_pos in case of recursion
        y_pos = y_pos + 30;
        if(entry->extension == "Folder" && data_ptr->recursive_flag == true)
        {
            y_pos = recursiveRender(entry->path, y_pos, starting_x+15, renderer, data_ptr);
        }

        //update variables
        thin_rect.y = y_pos + 28;
        thin_rect.x = starting_x;
        thin_rect.w = 360-starting_x;
        image_rect.y = y_pos;
        text_rect.y = y_pos;
        image_rect.y = y_pos;
        size_rect.y = y_pos;
        permissions_rect.y = y_pos;
    }
    return y_pos;
}


//use as input the size in bytes and convert to the easiest readable size
double formatSize(int size)
{
    double result = size;
    while (result >= 1024)
    {
        result = result / 1024;
    }
    result = std::ceil(result * 100.0) / 100.0;
    return result;
}//formatSize

std::string getSizeSuffix(int size){
    int divCount = 0;
    while (std::round(size/1024) >= 1)
    {
        size = size / 1024;
        divCount++;
    }
    if(divCount == 0){
        return "B";
    }
    else if(divCount == 1){
        return "KB";
    }
    else if(divCount == 2){
        return "MB";
    }
    else {
        return "GB";
    }
}//getSizeSuffix


//use a directory path to create files for everything in the directory
std::vector<File*> createFile(std::string path, AppData *data_ptr, SDL_Renderer *renderer)
{
    //directory for all files in it
    std::vector<File*> final_directory;
    //loop through all files in the directory using path
    for(const auto & entry : std::filesystem::directory_iterator(path))
    {
        // variables
        std::string ex;
        std::string name;
        int size;
        SDL_Color color = { 0, 0, 0 };

        //get the entry and make it a string for ease of use
        std::string clean_entry = entry.path().string();
        clean_entry = clean_entry.substr(clean_entry.find_last_of("/")+1);
        //check for valid file
        if(!(clean_entry.substr(0,1) == "." && clean_entry.substr(0, 2) != ".."))
        {
           //if directory
           if(clean_entry.find(".") == std::string::npos && std::filesystem::is_directory(entry) )
            {
                //variables
                Directory *currfile = new Directory();
                SDL_Surface *surface;
                
                //path
                currfile->path = entry.path();
                
                //icon
                currfile->texture_icon = data_ptr->folder;
                
                //extension texture
                currfile->extension = "Folder";
                surface = TTF_RenderText_Solid(data_ptr->font, currfile->extension.c_str(), color);
                currfile->texture_extension = SDL_CreateTextureFromSurface(renderer, surface);

                //filename
                currfile->filename = clean_entry;
                if(clean_entry.length() > 20)
                {
                    std::string modified_name = clean_entry.substr(0,20) + "...";
                    surface = TTF_RenderText_Solid(data_ptr->font, modified_name.c_str(), color);
                }
                else
                {
                    surface = TTF_RenderText_Solid(data_ptr->font, currfile->filename.c_str(), color);
                }
                currfile->texture_name = SDL_CreateTextureFromSurface(renderer, surface);
                
                
                if(data_ptr->recursive_flag == true)
                {
                    currfile->contents = createFile( currfile->path, data_ptr, renderer);
                }
                
                SDL_FreeSurface(surface);
                final_directory.push_back(currfile);
            }
            else
            {
                //variables
                File *currfile = new File();
                SDL_Surface *surface;

                //extension
                ex = clean_entry.substr(clean_entry.find_last_of(".")+1);
                surface = TTF_RenderText_Solid(data_ptr->font, currfile->extension.c_str(), color);
                currfile->texture_extension = SDL_CreateTextureFromSurface(renderer, surface);

                //filename
                currfile->filename = clean_entry;
                if(clean_entry.length() > 20)
                {
                    std::string modified_name = clean_entry.substr(0,20) + "...";
                    surface = TTF_RenderText_Solid(data_ptr->font, modified_name.c_str(), color);
                }
                else
                {
                    surface = TTF_RenderText_Solid(data_ptr->font, currfile->filename.c_str(), color);
                }
                currfile->texture_name = SDL_CreateTextureFromSurface(renderer, surface);

                //texture_icon
                if(ex == "jpg" || ex == "jpeg" || ex == "png" || ex == "tif" || ex == "tiff" || ex == "gif")
                {
                    currfile->extension = "Image";
                    currfile->texture_icon = data_ptr->image;
                }
                else if(ex == "mp4" || ex == "mov" || ex == "mkv" || ex == "avi" || ex == "wem" )
                {
                    currfile->extension = "Video";
                    currfile->texture_icon = data_ptr->video;
                }
                else if(ex == "h" || ex == "c" || ex == "cpp" || ex == "py" || ex == "java" || ex == "js")
                {
                    currfile->extension = "Code";
                    currfile->texture_icon = data_ptr->code;
                }
                else if(isExecutable(entry.path()))
                {
                    currfile->extension = "Executable";
                    currfile->texture_icon = data_ptr->executable;
                }
                else{ 
                    currfile->extension = "Other";
                    currfile->texture_icon = data_ptr->other; 
                }

                //path
                currfile->path = entry.path();

                //permissions
                currfile->permissions = FilePermissions(currfile->path);
                surface = TTF_RenderText_Solid(data_ptr->font, currfile->permissions.c_str(), color);
                currfile->texture_permissions = SDL_CreateTextureFromSurface(renderer, surface);

                //size
                currfile->size = std::filesystem::file_size(entry);
                currfile->size_suffix = getSizeSuffix(currfile->size);
                double formatted_size = formatSize(currfile->size);
                std::string string_size = std::to_string(formatted_size);
                string_size.append(" ");
                string_size.append(currfile->size_suffix);
                
                surface = TTF_RenderText_Solid(data_ptr->font, string_size.c_str(), color);
                currfile->texture_size = SDL_CreateTextureFromSurface(renderer, surface);
                
                SDL_FreeSurface(surface);
                final_directory.push_back(currfile);
            }

        }
    
    }
    std::sort(final_directory.begin(), final_directory.end(), FileSortComparator); 
    return final_directory;
}

void EventHandler(SDL_Event *event, AppData *data_ptr, SDL_Renderer *renderer)
{
    //variables
    std::vector<File*> file_list = data_ptr->files;
    int type = 2; //type of event (folder - 0 or file - 1)
    File* entry;
    pid_t process;

    //arrow down button
    if(event->button.button == SDL_BUTTON_LEFT && 
        event->button.x >= 2 &&
        event->button.x <= 23 &&
        event->button.y >= 550 &&
        event->button.y <= 590)
        {
            data_ptr->top = data_ptr->top + 1;
            render(renderer, data_ptr);
        }
    //arrow up button
    else if(event->button.button == SDL_BUTTON_LEFT && 
        event->button.x >= 2 &&
        event->button.x <= 23 &&
        event->button.y >= 55 &&
        event->button.y <= 95 &&
        data_ptr->top != 0)
        {
            data_ptr->top = data_ptr->top - 1;
            render(renderer, data_ptr);
        }
    else if( event->button.button == SDL_BUTTON_LEFT && 
        event->button.x >= 250 &&
        event->button.x <= 395 &&
        event->button.y >= 20 &&
        event->button.y <= 53)
        {
            data_ptr->recursive_flag = !data_ptr->recursive_flag;
            render(renderer, data_ptr);
        }
    else{
    //check the x and y of the mouse if it is within a text rectangle
        for(int i = 0; i < file_list.size(); i++)
        {   
            File* temp_entry = file_list.at(i);
            //if the press is within the rectangle of a file or directory
            if(event->button.button == SDL_BUTTON_LEFT && 
                event->button.x >= temp_entry->x &&
                event->button.x <= (temp_entry->x + temp_entry->w) &&
                event->button.y >= temp_entry->y &&
                event->button.y <= (temp_entry->y + temp_entry->h))
                {
                    if(temp_entry->extension == "Folder")
                    {
                        type = 0;
                    }
                    else
                    {
                        type = 1;
                    }
                    entry = temp_entry;
                }
            
        }//for
    }//else
    //if it clicked on a folder or text
    if(type == 0 || type == 1)
    {
        // clicked on folder
        if(type == 0)
        {
            //std::cout << entry->extension << std::endl;
            data_ptr->files = createFile( entry->path, data_ptr, renderer);
            data_ptr->top = 0;
            data_ptr->curr_folder_path = entry->path;
            render(renderer, data_ptr);
        }
        // clicked on file
        else
        {
            std::cout << entry->extension << std::endl;
            //open or execute file
            if(entry->extension == "Executable")
            {
                process = fork();
                // Child process
                if(process == 0)
                {
                    std::vector<char*> args;
                    args.push_back((char*)".");
                    args.push_back((char*)entry->path.c_str());
                    args.push_back(0);
                    execv(entry->path.c_str(), &args.front());
                    exit(0);
                
                }//if
                else
                {
                    wait(NULL);
                }//else
            }
            else
            {
                system(("xdg-open " + entry->path).c_str());
            }
        }
    }
}

std::string FilePermissions(std::string path)
{
    std::string result = "";
    //Modified from https://en.cppreference.com/w/cpp/filesystem/perms
    std::filesystem::perms p = std::filesystem::status(path).permissions();
     result.append((p & std::filesystem::perms::owner_read) != std::filesystem::perms::none ? "r" : "-");
     result.append((p & std::filesystem::perms::owner_write) != std::filesystem::perms::none ? "w" : "-");
     result.append((p & std::filesystem::perms::owner_exec) != std::filesystem::perms::none ? "x" : "-");
     result.append((p & std::filesystem::perms::group_read) != std::filesystem::perms::none ? "r" : "-");
     result.append((p & std::filesystem::perms::group_write) != std::filesystem::perms::none ? "w" : "-");
     result.append((p & std::filesystem::perms::group_exec) != std::filesystem::perms::none ? "x" : "-");
     result.append((p & std::filesystem::perms::others_read) != std::filesystem::perms::none ? "r" : "-");
     result.append((p & std::filesystem::perms::others_write) != std::filesystem::perms::none ? "w" : "-");
     result.append((p & std::filesystem::perms::others_exec) != std::filesystem::perms::none ? "x" : "-");
    return result;

}


bool isExecutable(std::string path)
{
    std::string result = "";
    std::filesystem::perms p = std::filesystem::status(path).permissions();
    result.append((p & std::filesystem::perms::owner_exec) != std::filesystem::perms::none ? "x" : "-");
    result.append((p & std::filesystem::perms::group_exec) != std::filesystem::perms::none ? "x" : "-");
    result.append((p & std::filesystem::perms::others_exec) != std::filesystem::perms::none ? "x" : "-");
    if(result.find("x") != std::string::npos)
    {
        return true;
    }
    return false;
}
