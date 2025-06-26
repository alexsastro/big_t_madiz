#include "lodepng.c"
#include "stdio.h"
#include "stdlib.h"
#include <dirent.h>
#include <string.h>

typedef struct {
    unsigned char br;
    int x;
    int y;
    int region;
    int visited;
} Pixel;

unsigned char get_brightness(unsigned char *image, int x, int y, unsigned w, unsigned h) {
    int idx = 4 * x + 4 * w * y;
    return (image[idx] + image[idx + 1] + image[idx + 2]) / 3;
}

int main(int argc, char *argv[]) {
    unsigned error;
    unsigned char* image;
    unsigned width, height;
    //char *dir = "./madizs/";
    struct dirent *entry;
    DIR *dp;

    /*dp = opendir("./madizs");
    if (dp == NULL) {
        perror("opendir");
        return EXIT_FAILURE;
    }
    const char *filename;
    while ((entry = readdir(dp)) != NULL) {
        printf("%s\n", entry->d_name);
	*/
	//printf("%s\n",strcat(dir, entry->d_name));
   	//filename = strcat("./madizs/", entry->d_name);
    const char* filename = argc > 1 ? argv[1] : "pich.png";
    int diff = 6;

    error = lodepng_decode32_file(&image, &width, &height, filename);
    if(error) printf("decoder error %u: %s\n", error, lodepng_error_text(error));

    Pixel **pixels = malloc(height * sizeof(Pixel*));
    for (int y = 0; y < height; y++) {
        pixels[y] = malloc(width * sizeof(Pixel));
        for (int x = 0; x < width; x++) {
            pixels[y][x].x = x;
            pixels[y][x].y = y;
            pixels[y][x].br = get_brightness(image, x, y, width, height);
            pixels[y][x].region = -1;
            pixels[y][x].visited = 0;
        }
    }

    int region_count = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (pixels[y][x].visited) continue;
            
            region_count++;
            int queue_size = width * height;
            int *queue_x = malloc(queue_size * sizeof(int));
            int *queue_y = malloc(queue_size * sizeof(int));
            int front = 0, rear = 0;
            
            queue_x[rear] = x;
            queue_y[rear] = y;
            rear++;
            pixels[y][x].visited = 1;
            pixels[y][x].region = region_count;
            
            while (front < rear) {
                int cx = queue_x[front];
                int cy = queue_y[front];
                front++;
                
                int dx[] = {0, 0, -1, 1};
                int dy[] = {-1, 1, 0, 0};
                
                for (int i = 0; i < 4; i++) {
                    int nx = cx + dx[i];
                    int ny = cy + dy[i];
                    

                    if (nx < 0 || nx >= width || ny < 0 || ny >= height || pixels[ny][nx].visited) 
                        continue;
                    
                    if (abs(pixels[cy][cx].br - pixels[ny][nx].br) <= diff) {
                        pixels[ny][nx].visited = 1;
                        pixels[ny][nx].region = region_count;
                        
                        queue_x[rear] = nx;
                        queue_y[rear] = ny;
                        rear++;
                    }
                }
            }
            
            free(queue_x);
            free(queue_y);
        }
    }


    unsigned char *imgout = malloc(4 * width * height);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = 4 * x + 4 * width * y;
            int region = pixels[y][x].region;
            
            int is_boundary = 0;
            int dx[] = {0, 0, -1, 1};
            int dy[] = {-1, 1, 0, 0};
            
            for (int i = 0; i < 4; i++) {
                int nx = x + dx[i];
                int ny = y + dy[i];
                
                if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                if (pixels[ny][nx].region != region) {
                        is_boundary = 1;
                        break;
                    }
                }
            }
            if (is_boundary) {
                imgout[idx] = 0;
                imgout[idx + 1] = 0;
                imgout[idx + 2] = 0;
            } else{
            imgout[idx] = (region * 50) % 256;
            imgout[idx + 1] = (region * 100) % 256;
            imgout[idx + 2] = (region * 150) % 256;
	    }
            imgout[idx + 3] = 255;
        }
    }

    lodepng_encode32_file("out_pic.png", imgout, width, height);
    
    for (int y = 0; y < height; y++) free(pixels[y]);
    free(pixels);
    free(imgout);
    free(image);
    
    printf("Segmentation complete! Found %d regions\n", region_count);
    
    //closedir(dp);
    //return EXIT_SUCCESS;
    //}
    return 0;
}

