string break_line = "";
                for (auto &it : current_line)
                {
                    break_line += it;
                    if (break_line.size() > cell_width / 6)
                    {
                        lines.push_back(break_line);
                        break_line = "";
                    }
                }
                current_line = break_line;
                lines.push_back(current_line); // Push the current line if it exceeds the width
                current_line = word;


                if (i == 0) {
            HPDF_Page_SetRGBFill(page, light_pink[0], light_pink[1], light_pink[2]);
            HPDF_Page_Rectangle(page, x_position, y_position - max_cell_height, cell_width, max_cell_height);
            HPDF_Page_Fill(page); // Fill the rectangle with light pink
        }