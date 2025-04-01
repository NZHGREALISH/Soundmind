void draw_question_mark(int x, int y)
{
    draw_orange_circle(x, y);
    int cx = x + 12;
    int cy = y + 12;
    int arc_cx = cx;
    int arc_cy = cy-3;  // move up 5 pixels
    draw_thick_arc(arc_cx, arc_cy, /*outer_r=*/6, /*thick=*/3,
                   /*start_deg=*/180.0f, /*end_deg=*/360.0f,
                   /*color=*/0x0000 /*BLACK*/);

    draw_thick_vertical_line(/*x=*/cx + 5,
                             /*y_start=*/cy - 3,
                             /*length=*/4,
                             /*thickness=*/3,
                             /*color=*/0x0000 /*BLACK*/);

    draw_thick_horizontal_line(/*y=*/(cy + 1),
                               /*x_start=*/(cx),
                               /*length=*/5,
                               /*thickness=*/3,
                               /*color=*/0x0000 /*BLACK*/);

    draw_thick_vertical_line(/*x=*/(cx),
                             /*y_start=*/(cy + 1),
                             /*length=*/5,
                             /*thickness=*/3,
                             /*color=*/0x0000 /*BLACK*/);

    {
        int dot_r = 2;
        int dot_cx = cx;
        int dot_cy = cy + 9;
        for (int dy = -dot_r; dy <= dot_r; dy++) {
            for (int dx = -dot_r; dx <= dot_r; dx++) {
                if (dx*dx + dy*dy <= dot_r*dot_r) {
                    plot_pixel(dot_cx + dx, dot_cy + dy, 0x0000 /*BLACK*/);
                }
            }
        }
    }
}