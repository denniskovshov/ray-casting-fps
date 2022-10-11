#include <iostream>
#include <fstream> // ofstream
#include <vector>
#include <cstdint> // what's inside?
#include <cassert> // assert
#include <cmath> // cos, sin
#include <sstream> // what's inside?
#include <iomanip> // what's inside?

typedef uint32_t color;
typedef uint8_t pixel;

color pack_color(const pixel red, const pixel green, const pixel blue, const pixel alpha=255) {
    auto result_color = (alpha << 24) + (blue << 16) + (green << 8) + red;
    return result_color;
}

void unpack_color(const color &color, pixel &red, pixel &green, pixel &blue, pixel &alpha) {
    red = (color >> 0) & 255;
    green = (color >> 8) & 255;
    blue = (color >> 16) & 255;
    alpha = (color >> 24) & 255;
}

void drop_ppm_image(const std::string filename, const std::vector<color> &image, const size_t width, const size_t height) {
    assert(image.size() == width * height);

    std::ofstream file_stream { filename, std::ios::binary };
    file_stream << "P6\n" << width << " " << height << "\n255\n";

    for (size_t i = 0; i < width * height; ++i) {
        pixel red, green, blue, alpha;
        unpack_color(image[i], red, green, blue, alpha);

        file_stream << static_cast<char>(red) << static_cast<char>(green) << static_cast<char>(blue);
    }

    file_stream.close();
}

void draw_rect(std::vector<color> &img, const size_t img_width, const size_t img_height, const size_t rect_x, const size_t rect_y, const size_t rect_width, const size_t rect_height, const color rect_color) {
    assert(img.size() == img_width * img_height);

    for (size_t i = 0; i < rect_width; i++) {
        for (size_t j = 0; j < rect_height; j++) {
            size_t x = rect_x + i;
            size_t y = rect_y + j;

            //assert(x < img_width && y < img_height);
            if (x >= img_width || y >= img_height)
                continue;

            img[x + y * img_width] = rect_color;
        }
        
    }
}

int main() {
    const size_t img_width = 1024; // to incorp 2 images
    const size_t img_height = 512;

    // vector of certain capacity, all elements are 255 default value
    // std::vector<color>* img_buffer = new std::vector<color> { img_width * img_height, 255 };
    // std::vector<color>& img_buffer_ref = *img_buffer;
    // TODO: incorrectly allocates for entire image, causes SEGFAULT, try replacing 255 with pack_color(255...)
    std::vector<color> img_buffer_ref(img_width * img_height, pack_color(255, 255, 255));

    // random colors for better highlighting
    const size_t color_cnt = 10;
    std::vector<color> colors(color_cnt);
    
    for (size_t i = 0; i < color_cnt; i++) {
        colors[i] = pack_color(rand()%255, rand()%255, rand()%255);
    }

    const size_t map_width = 16;
    const size_t map_height = 16;
    // our game map
    const char map[] = "0000222222220000"\
                       "1              0"\
                       "1      11111   0"\
                       "1     0        0"\
                       "0     0  1110000"\
                       "0     3        0"\
                       "0   10000      0"\
                       "0   0   11100  0"\
                       "0   0   0      0"\
                       "0   0   1  00000"\
                       "0       1      0"\
                       "2       1      0"\
                       "0       0      0"\
                       "0 0000000      0"\
                       "0              0"\
                       "0002222222200000";
    
    // +1 for the null terminated string
    assert(sizeof(map) == map_width * map_height + 1);

    // data is stored in 1D as, [(1st row),(2nd row),(3rd row)...] 
    // fill the screen with color gradients
    /*
    for (size_t j = 0; j < img_height; j++) {
        for (size_t i = 0; i < img_width; i++) {
            // varies between 0 and 255 as j sweeps the vertical
            //pixel red = 255 * j / double(img_height);
            // varies between 0 and 255 as i sweeps the horizontal
            //pixel green = 255 * i / double(img_width);
            //pixel blue = 0;

            // TODO: change back once allocation is fixed
            //auto location = i + j * img_width;
            //img_buffer_ref[location] = pack_color(red, green, blue);
            //auto color = pack_color(red, green, blue);
            auto color = pack_color(255, 255, 255);
            img_buffer_ref.push_back(color);
        }
    }
    */
    
    const size_t rect_width = img_width / (map_width * 2);
    const size_t rect_height = img_height / map_height;

    // draw the player on the map
    double player_x = 3.456; // 12.544 157.351936
    double player_y = 2.345; // 13.655 186.459025
    double player_dir = 1.523;
    // TODO
    const double player_fov = M_PI / 3.; // field of view
    
    //auto player_color = pack_color(255, 255, 255);
    //draw_rect(img_buffer_ref, img_width, img_height, player_x * rect_width, player_y * rect_height, 5, 5, player_color);

    // 360 degree view for now
    for (size_t frame = 0; frame < 360; frame++) {
        std::stringstream img_frame_filename;
        img_frame_filename << std::setfill('0') << std::setw(5) << frame << ".ppm";

        // rotate view
        player_dir += 2 * M_PI / 360;

        // clear the screen
        img_buffer_ref = std::vector<color>(img_width * img_height, pack_color(255, 255, 255));

        // re-draw the map
        for (size_t j = 0; j < map_height; j++) {
            for (size_t i = 0; i < map_width; i++) {
                // skip empty spaces
                if (map[i + j * map_width] == ' ')
                    continue;
                
                size_t rect_x = i * rect_width;
                size_t rect_y = j * rect_height;

                size_t color_idx = map[i + j * map_width] - '0';
                assert(color_idx < color_cnt);

                auto color = colors[color_idx];
                draw_rect(img_buffer_ref, img_width, img_height, rect_x, rect_y, rect_width, rect_height, color);
            }   
        }

        // draw the visibility cone AND the "3D" view
        for (size_t i = 0; i < img_width / 2; i++) {
            double angle = player_dir - player_fov / 2. + player_fov * i / double(img_width / 2);

            // TODO: explain 't'
            // player 1 ray of view
            for (double view_len = 0; view_len < 10; view_len += 0.01) {
                double x = player_x + view_len * std::cos(angle);
                double y = player_y + view_len * std::sin(angle);

                // if (map[int(x) + int(y) * map_width] != ' ')
                //     break;

                size_t pix_x = x * rect_width;
                size_t pix_y = y * rect_height;

                // visibility cone
                img_buffer_ref[pix_x + pix_y * img_width] = pack_color(160, 160, 160);

                // the "3D" view
                // our ray touches a wall, so draw the vertical column to create an illusion of 3D
                if (map[int(x) + int(y) * map_width] != ' ') {

                    size_t color_idx = map[int(x) + int(y) * map_width] - '0';
                    assert(color_idx < color_cnt);

                    // with fish eye effect
                    size_t col_height = img_height / view_len;
                    // withOUT fish eye effect
                    //size_t col_height = img_height / (view_len * std::cos(angle - player_dir));

                    auto color = colors[color_idx];
                    draw_rect(img_buffer_ref, img_width, img_height, img_width / 2 + i, img_height / 2 - col_height / 2, 1, col_height, color);

                    break;
                }
            }
        }

        drop_ppm_image(img_frame_filename.str(), img_buffer_ref, img_width, img_height);
    }

    return 0;
}