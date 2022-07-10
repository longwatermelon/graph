in vec3 i_pos;
in int i_x;
in int i_y;
out vec3 f_color;

void main()
{
    vec3 gr_pos = i_pos;
    f_color = vec3(i_x, i_y, 100);
}

