#include<stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <cairo.h>
#include <gtk/gtk.h>

#define M_PI		3.14159265358979323846//something wrong with math.h
#define BOARD_WIDTH 17
#define BOARD_HEIGHT 16
#define WINDOW_WIDTH 550
#define WINDOW_HEIGHT 440
#define COLOURS 6
#define ARROW_LENGTH 60
#define STAY_BALL_X_POS 223
#define STAY_BALL_Y_POS 422
#define BALL_RADIUS 12
#define DRAW_FRQ 14
#define DOWN_FRQ 5
#define SMALL_WINDOW_WIDTH 200

int BALL_COL=3;//amount of the colours, range from 1 to 6
int i, j, order=0, new_ball_colour, board_number, balls_amount, level_number=1, score=0, amount, shot=0, disap1=0, disap2=0, max_balls;
int shoot_to_i, shoot_to_j, balls_down, max_balls_place, end_window_shown=0, shot_balls;//shot_balls- balls which has been shot
char score_char[8], level_char[4], a;
double m, n, k, l, x1=STAY_BALL_X_POS, x2=STAY_BALL_X_POS, y1=STAY_BALL_Y_POS, y2=STAY_BALL_Y_POS, angle, x=STAY_BALL_X_POS, y=STAY_BALL_Y_POS;
double x_arrow, y_arrow, x_clicked, y_clicked, d,  delta, A, B, moving_ball_x, moving_ball_y;
double red[COLOURS+1] = {0,0,255,5,30,0,0}, green[COLOURS+1] = {0,20,0,20,0,0,50}, blue[COLOURS+1] = {0,10,0,0,10,40,0};
FILE *file;
int data_error=0, balls_down_read=0;

struct board
{
  int visible;
  int colour;
  double x_position;
  double y_position;
  int edge;
} ball[BOARD_WIDTH][BOARD_HEIGHT];

GtkWidget *window, *darea, *button_box, *draw_alignment, *all_box, *exit_button, *new_game_button, *score_label1, *level_label1;
GtkWidget *score_label2, *level_label2;
GtkWidget *end_window, *end_label1, *end_label2, *end_button1, *end_button2, *end_container;
GtkWidget *save_b, *save_t, *read_b, *read_t;

void do_drawing(cairo_t *, GtkWidget *);
void on_draw_event(GtkWidget *, cairo_t *);
void mouse_movement();//after signal from a mouse
gboolean repeat();
void create_board(), create_main_window(), nothing(), new_level();
void clicked(GtkWidget *widget, GdkEventButton *event);
void check_edge(int i, int j), find_place(), new_game(), game_over(), create_end_window();
void ball_disap1(int i, int j), ball_disap2 (int i, int j, int edge);
void save_binary(), save_text(), read_binary(), read_text(), read_error(), check_read();

int main (int argc, char *argv[])
{
    gtk_init(&argc, &argv);
    srand(time(NULL));

    create_main_window();
    create_board();
    create_end_window();

    g_signal_connect(G_OBJECT(new_game_button), "clicked", G_CALLBACK(new_game), NULL);
    g_signal_connect(G_OBJECT(exit_button), "clicked", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(G_OBJECT(darea), "draw", G_CALLBACK(on_draw_event), NULL);//starting the drawing
    g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(nothing), NULL);

    g_signal_connect(G_OBJECT(end_button1), "clicked", G_CALLBACK(new_game), NULL);
    g_signal_connect(G_OBJECT(end_button2), "clicked", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(G_OBJECT(end_window), "delete-event", G_CALLBACK(nothing), NULL);

    gtk_widget_set_events(darea, GDK_POINTER_MOTION_MASK);//making motion-notify-event work
    g_signal_connect(darea, "motion-notify-event", G_CALLBACK(mouse_movement), NULL);//the signal emitted when a mouse is moved
    gtk_widget_add_events(darea, GDK_BUTTON_PRESS_MASK);//making button-press-event work
    g_signal_connect(darea, "button-press-event", G_CALLBACK(clicked), NULL);//the signal emitted when a signal from a mouse is given

    g_signal_connect(G_OBJECT(save_b), "clicked", G_CALLBACK(save_binary), NULL);
    g_signal_connect(G_OBJECT(save_t), "clicked", G_CALLBACK(save_text), NULL);
    g_signal_connect(G_OBJECT(read_b), "clicked", G_CALLBACK(read_binary), NULL);
    g_signal_connect(G_OBJECT(read_t), "clicked", G_CALLBACK(read_text), NULL);

    g_timeout_add(20, repeat, NULL);
    gtk_widget_show_all(window);
  gtk_main();

  return 0;
}

void on_draw_event(GtkWidget *widget, cairo_t *cr )
{
  do_drawing(cr, widget);
}//that function starts the drawing function

void do_drawing(cairo_t *cr, GtkWidget *widget)
{
    //background colours:
    cairo_set_line_width(cr, 1);
    cairo_set_source_rgb(cr, 0.94, 0.94, 0.94);
    cairo_rectangle(cr, 0, 0, 450, 440);
    cairo_stroke_preserve(cr);
    cairo_fill(cr);

    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
    cairo_rectangle(cr, 5, 5, 440, 365);
    cairo_stroke_preserve(cr);
    cairo_set_source_rgb(cr, 0.92, 0.92, 0.92);
    cairo_fill(cr);

    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
    cairo_rectangle(cr, 5, 5, 440, 24*balls_down);
    cairo_stroke_preserve(cr);
    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
    cairo_fill(cr);

    //creating the board of balls:
    for(i=0 ;i<BOARD_HEIGHT;i++)
        for(j=0 ;j<BOARD_WIDTH;j++)
        {
            if(ball[j][i].visible)
            {
                cairo_set_line_width(cr, 1);
                cairo_set_source_rgb(cr, 0, 0, 0);//black contour

                cairo_arc(cr, ball[j][i].x_position, ball[j][i].y_position, BALL_RADIUS, 0, 2 * M_PI);//creating a black circle
                cairo_stroke_preserve(cr);//drawing the circle

                cairo_set_source_rgb(cr, red[ball[j][i].colour], green[ball[j][i].colour], blue[ball[j][i].colour]);
                cairo_fill(cr);//filling the circle
            }
            /*if(ball[j][i].edge==2)
            {
                cairo_set_line_width(cr, 1);
                cairo_set_source_rgb(cr, 0, 0, 0);//black contour

                cairo_arc(cr, ball[j][i].x_position, ball[j][i].y_position, BALL_RADIUS, 0, 2 * M_PI);//creating a black circle
                cairo_stroke_preserve(cr);//drawing the circle
                cairo_fill(cr);//filling the circle
            }*/
        }

        if(order==0)
          {
           new_ball_colour=1+rand()%BALL_COL;
          }
        cairo_set_line_width(cr, 1);
        cairo_set_source_rgb(cr, 0, 0, 0);//black contour

        if(shot)
        {
            if(x_clicked>STAY_BALL_X_POS-6 && x_clicked<STAY_BALL_X_POS+6)
                cairo_arc(cr, STAY_BALL_X_POS, moving_ball_y, BALL_RADIUS, 0, 2 * M_PI);//creating a black circle
                else
                cairo_arc(cr, moving_ball_x, A*moving_ball_x+B, BALL_RADIUS, 0, 2 * M_PI);//creating a black circle
        }
        else
        cairo_arc(cr, STAY_BALL_X_POS, STAY_BALL_Y_POS, BALL_RADIUS, 0, 2 * M_PI);//creating a black circle

        cairo_stroke_preserve(cr);//drawing the circle
        cairo_set_source_rgb(cr,  red[new_ball_colour], green[new_ball_colour], blue[new_ball_colour]);
        cairo_fill(cr);//filling the circle

        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_set_line_width(cr, 0.5);
        cairo_move_to(cr, STAY_BALL_X_POS, STAY_BALL_Y_POS);//the beginning of the line segment
        cairo_line_to(cr, x, y);//the end of the line segment
        cairo_stroke_preserve(cr);//drawing the arrow

        cairo_move_to(cr, x, y);
        cairo_line_to(cr, x1, y1);
        cairo_stroke_preserve(cr);

        cairo_move_to(cr, x, y);
        cairo_line_to(cr, x2, y2);
        cairo_stroke_preserve(cr);

        order=1;
}//drawing function
void mouse_movement(GtkWidget *widget, GdkEvent *event)
{
    if (event->type==GDK_MOTION_NOTIFY)
    {
        GdkEventMotion* e=(GdkEventMotion*)event;
        if(e->y < 367 && e->x < 446 && e->x >5 && e->y > 5)
        {x_arrow=e->x;
        y_arrow=e->y;
        }
    }
}

gboolean repeat()
{
//arrow:
        l=x_arrow-STAY_BALL_X_POS;
        k=STAY_BALL_Y_POS-y_arrow;
        n=ARROW_LENGTH*l/sqrt(k*k+l*l);
        m=ARROW_LENGTH*k/sqrt(k*k+l*l);
        x=STAY_BALL_X_POS+n;
        y=STAY_BALL_Y_POS-m;

        angle = atan2 (y - STAY_BALL_Y_POS, x - STAY_BALL_X_POS ) + M_PI;

        x1 = x + 20 * cos(angle - M_PI/6);
        y1 = y + 20 * sin(angle - M_PI/6);
        x2 = x + 20 * cos(angle + M_PI/6);
        y2 = y + 20 * sin(angle + M_PI/6);

//moving ball:
        if(shot)
            {
                if(x_clicked>STAY_BALL_X_POS-6 && x_clicked<STAY_BALL_X_POS+6)
                    moving_ball_y-=DRAW_FRQ;
                    else
                    moving_ball_x+=delta;

                if(shoot_to_i!=-1)
                if((x_clicked!=STAY_BALL_X_POS
                   && ((moving_ball_x>ball[shoot_to_j][shoot_to_i].x_position && A<0)
                   || (moving_ball_x<ball[shoot_to_j][shoot_to_i].x_position && A>0)
                   || (moving_ball_x*A+B<ball[shoot_to_j][shoot_to_i].y_position)))
                   ||(x_clicked==STAY_BALL_X_POS && moving_ball_y<=ball[shoot_to_j][shoot_to_i].y_position))
                    {
                        shot_balls++;
                        balls_down=shot_balls/DOWN_FRQ+balls_down_read;
                        shot=0;
                        balls_amount++;
                        ball[shoot_to_j][shoot_to_i].visible=1;
                        ball[shoot_to_j][shoot_to_i].edge=1;
                        ball[shoot_to_j][shoot_to_i].colour=new_ball_colour;
                        order=0;
                        gtk_widget_set_sensitive(new_game_button, TRUE);
                        gtk_widget_set_sensitive(save_b, TRUE);
                        gtk_widget_set_sensitive(save_t, TRUE);
                        gtk_widget_set_sensitive(read_b, TRUE);
                        gtk_widget_set_sensitive(read_t, TRUE);
                        if(shoot_to_i>max_balls_place)
                            max_balls_place=shoot_to_i;

                        ball_disap1(shoot_to_i, shoot_to_j);//checking which balls (worth 1 point) should disappear
                          if(disap1>=3)
                            {for(j=0;j<BOARD_WIDTH;j++)
                            for(i=0;i<BOARD_HEIGHT;i++)
                            if(ball[j][i].edge==3)
                            {
                                ball[j][i].visible=0;
                                ball[j][i].colour=0;
                            }

                            for(i=0;i<BOARD_HEIGHT;i++)
                            for(j=0;j<BOARD_WIDTH;j++)
                            {
                                //edge=4 when balls should disappear and they are worth 2 points
                                //edge=5 when balls should not disappear
                                if(ball[j][i].edge!=4 && ball[j][i].edge!=5  && ball[j][i].visible==1)
                                    {if(i==0)
                                    {ball[j][i].edge=5;
                                    ball_disap2(i, j, 5);}
                                    else
                                    {ball[j][i].edge=4;
                                    disap2++;
                                    ball_disap2(i, j, 4);}
                            }       }

                            for(j=0;j<BOARD_WIDTH;j++)
                            for(i=0;i<BOARD_HEIGHT;i++)
                            if(ball[j][i].edge==4)
                            {
                                ball[j][i].visible=0;
                                ball[j][i].colour=0;
                            }
                            disap1++;
                            score=score+disap1+2*disap2;
                            }
                         if(disap1>=3)
                         {
                            balls_amount=balls_amount-disap1-disap2;
                            max_balls_place=0;
                            for(j=0;j<BOARD_WIDTH;j++)
                            for(i=0;i<BOARD_HEIGHT;i++)
                                if(ball[j][i].visible==1 && i>max_balls_place)
                                max_balls_place=i;
                         }
                        itoa(score, score_char, 10);
                        gtk_label_set_text(GTK_LABEL(score_label2), score_char);//changing the label
                        disap1=0;
                        disap2=0;

                        for(j=0;j<BOARD_WIDTH;j++)
                        for(i=0;i<BOARD_HEIGHT;i++)
                            ball[j][i].edge=0;

                        check_edge(i, j);

                        shoot_to_i=-1;
                        shoot_to_j=-1;
                        if(shot_balls%DOWN_FRQ==0)
                            {for(j=0;j<BOARD_WIDTH;j++)
                            for(i=0;i<BOARD_HEIGHT;i++)
                            ball[j][i].y_position+=24;
                            max_balls--;
                            }

                        if(balls_amount==0)
                            new_level();

                        if(max_balls_place>max_balls)
                            game_over();
                    }

                if(shoot_to_i<0)
                {
                if(moving_ball_x<=22)
                {
                    A=-A;
                    B=B-2*A*22;
                    delta=-delta;
                    find_place();
                }
                else
                if(moving_ball_x>=433)
                {
                    A=-A;
                    B=B-2*A*433;
                    delta=-delta;
                    find_place();
                }}

                if(A*moving_ball_x+B<5+balls_down*24 || (moving_ball_y<5+balls_down*24  && x_clicked>STAY_BALL_X_POS-6 && x_clicked<STAY_BALL_X_POS+6))
                {
                    shot=0;
                    gtk_widget_set_sensitive(new_game_button, TRUE);
                    gtk_widget_set_sensitive(save_b, TRUE);
                    gtk_widget_set_sensitive(save_t, TRUE);
                    gtk_widget_set_sensitive(read_b, TRUE);
                    gtk_widget_set_sensitive(read_t, TRUE);
                }//if sthg goes wrong
            }

    gtk_widget_queue_draw(darea);
        return 1;
}

void create_board()
{
    max_balls_place=0;
    file=fopen("balls_board.txt", "r");
    if(file==NULL)
    {
        printf("Opening file - error");
        gtk_main_quit();
    }
    board_number=1+rand()%15;//choosing the board
    for(i=1;i<board_number;i++)
    {
    fscanf(file, "%d%c", &j, &a);
    for(j=0;j<162;j++)
    {
    fscanf(file, "%c", &a);
    }
    }//missing not needed data (earlier boards)

    fscanf(file, "%d", &balls_amount);//number of balls at the beginning of the level
    for(i=0;i<153;i++)
    {
        fscanf(file, "%c", &a);
        switch(a)
        {
            case '\n': i--; break;
            case '1':
                    ball[i%BOARD_WIDTH][i/BOARD_WIDTH].visible=1;
                    ball[i%BOARD_WIDTH][i/BOARD_WIDTH].colour=1+rand()%BALL_COL;
                    ball[i%BOARD_WIDTH][i/BOARD_WIDTH].x_position=20+25*(i%BOARD_WIDTH)+BALL_RADIUS*((i/BOARD_WIDTH)%2);
                    ball[i%BOARD_WIDTH][i/BOARD_WIDTH].y_position=20+(i/BOARD_WIDTH)*24;
                    ball[i%BOARD_WIDTH][i/BOARD_WIDTH].edge=0;
                    if((i/BOARD_WIDTH)>max_balls_place)
                        max_balls_place=i/BOARD_WIDTH;
                    break;
            case '0':
                    ball[i%BOARD_WIDTH][i/BOARD_WIDTH].visible=0;
                    ball[i%BOARD_WIDTH][i/BOARD_WIDTH].x_position=20+25*(i%BOARD_WIDTH)+BALL_RADIUS*((i/BOARD_WIDTH)%2);
                    ball[i%BOARD_WIDTH][i/BOARD_WIDTH].y_position=20+(i/BOARD_WIDTH)*24;
                    ball[i%BOARD_WIDTH][i/BOARD_WIDTH].edge=0;
                    ball[i%BOARD_WIDTH][i/BOARD_WIDTH].colour=0;
                    break;
            default : printf("Error"); gtk_main_quit(); break;
        }
    }//creating the board
    for(i=153;i<272;i++)
    {
        ball[i%BOARD_WIDTH][i/BOARD_WIDTH].x_position=20+25*(i%BOARD_WIDTH)+BALL_RADIUS*((i/BOARD_WIDTH)%2);
        ball[i%BOARD_WIDTH][i/BOARD_WIDTH].y_position=20+(i/BOARD_WIDTH)*24;
        ball[i%BOARD_WIDTH][i/BOARD_WIDTH].edge=0;
        ball[i%BOARD_WIDTH][i/BOARD_WIDTH].visible=0;
        ball[i%BOARD_WIDTH][i/BOARD_WIDTH].colour=0;
    } //filling the information about 'invisible balls'

        check_edge(0,0);//information which balls are on the edge (1) and where balls can be put (2)
// check_edge(i,j);
    fclose(file);
    shot_balls=0;
    balls_down=0;
    balls_down_read=0;
    max_balls=14;
}

void create_main_window()
{
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_widget_set_size_request(window, WINDOW_WIDTH, WINDOW_HEIGHT);
    gtk_window_set_resizable (GTK_WINDOW(window), FALSE);//changing the size of the window is impossible
    gtk_window_set_title(GTK_WINDOW(window), "Bust a move");

    darea = gtk_drawing_area_new();//creating the drawing area
    draw_alignment=gtk_alignment_new(0, 0, 1, 1);
    gtk_widget_set_size_request (draw_alignment, 450, 440);
    gtk_container_add(GTK_CONTAINER(draw_alignment), darea);

    new_game_button=gtk_button_new_with_label("New game");
    gtk_widget_set_size_request (new_game_button, 95, 40);
    exit_button=gtk_button_new_with_label("Exit");
    gtk_widget_set_size_request (exit_button, 95, 40);

    save_b=gtk_button_new_with_label("Save (b)");
    gtk_widget_set_size_request (new_game_button, 95, 20);
    save_t=gtk_button_new_with_label("Save (t)");
    gtk_widget_set_size_request (exit_button, 95, 20);

    read_b=gtk_button_new_with_label("Read game (b)");
    gtk_widget_set_size_request (new_game_button, 95, 20);
    read_t=gtk_button_new_with_label("Read game (t)");
    gtk_widget_set_size_request (exit_button, 95, 20);

    score_label1=gtk_label_new("SCORE");
    level_label1=gtk_label_new("LEVEL");
    score_label2=gtk_label_new("0");//score should change properly
    level_label2=gtk_label_new("1");//level number should change properly

    button_box=gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    all_box=gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add(GTK_CONTAINER(window), all_box);

    gtk_box_pack_start(GTK_BOX(button_box), score_label2, 1, 0, 0);
    gtk_box_pack_start(GTK_BOX(button_box), level_label1, 1, 0, 0);
    gtk_box_pack_start(GTK_BOX(button_box), level_label2, 1, 0, 0);
    gtk_box_pack_start(GTK_BOX(button_box), new_game_button, 1, 0, 0);
    gtk_box_pack_start(GTK_BOX(button_box), save_b, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(button_box), save_t, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(button_box), read_b, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(button_box), read_t, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(button_box), exit_button, 1, 0, 0);
    gtk_box_pack_start(GTK_BOX(all_box), draw_alignment, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(all_box), button_box, 0, 0, 0);
}

void nothing()
{}

void clicked(GtkWidget *widget, GdkEventButton *event)
{
    if(shot==0 && event->y < 367 && event->x < 446 && event->x >5 && event->y > 5 && end_window_shown==0)
    {x_clicked=event->x;
    y_clicked=event->y;
    if(x_clicked>STAY_BALL_X_POS-6 && x_clicked<STAY_BALL_X_POS+6)
    {
    amount=(STAY_BALL_Y_POS-y_clicked)/DRAW_FRQ;
    shot=1;
    shoot_to_j=8;
    moving_ball_y=STAY_BALL_Y_POS;
    for(i=BOARD_HEIGHT-1;i>0;i--)
        if(ball[shoot_to_j][i].edge==2)
        shoot_to_i=i;
    }
    else
    {
    A=(y_clicked-STAY_BALL_Y_POS)/(x_clicked-STAY_BALL_X_POS);
    B=(STAY_BALL_Y_POS*x_clicked-STAY_BALL_X_POS*y_clicked)/(x_clicked-STAY_BALL_X_POS);
    shot=1;
    moving_ball_x=STAY_BALL_X_POS;

    shoot_to_i=-1;
    shoot_to_j=-1;

    find_place ();

    amount=sqrt((x_clicked-STAY_BALL_X_POS)*(x_clicked-STAY_BALL_X_POS)+(y_clicked-STAY_BALL_Y_POS)*(y_clicked-STAY_BALL_Y_POS))/DRAW_FRQ;
    delta=(x_clicked-STAY_BALL_X_POS)/amount;
}
gtk_widget_set_sensitive(new_game_button, FALSE);
gtk_widget_set_sensitive(save_b, FALSE);
gtk_widget_set_sensitive(save_t, FALSE);
gtk_widget_set_sensitive(read_b, FALSE);
gtk_widget_set_sensitive(read_t, FALSE);
}
}

void check_edge(int i, int j)
{
    for(j=0;j<BOARD_WIDTH;j++)
    for(i=0;i<BOARD_HEIGHT;i++)
    {if(ball[j][i].edge!=2 && ball[j][i].visible==1)
    {
    if(ball[j][i+1].visible==0)//ok
        {
            ball[j][i].edge=1;
            ball[j][i+1].edge=2;
        }

        if(j!=16 && ball[j+1][i].visible==0)//ok
        {
            ball[j][i].edge=1;
            ball[j+1][i].edge=2;
        }

        if(j!=0 && ball[j-1][i].visible==0)//ok
        {
            ball[j][i].edge=1;
            ball[j-1][i].edge=2;
        }

        if(i!=0 && ball[j][i-1].visible==0)
        {
            ball[j][i].edge=1;
            ball[j][i-1].edge=2;
        }

        if(i!=0 && i%2==1 && j!=16 && ball[j+1][i-1].visible==0)
        {
            ball[j][i].edge=1;
            ball[j+1][i-1].edge=2;
        }

        if(i%2==1 && j!=16 && ball[j+1][i+1].visible==0)
        {
            ball[j][i].edge=1;
            ball[j+1][i+1].edge=2;
        }

        if(i!=0 && j!=0 && i%2==0 && ball[j-1][i-1].visible==0)
        {
            ball[j][i].edge=1;
            ball[j-1][i-1].edge=2;
        }

        if(i!=16 && j!=0 && i%2==0 && ball[j-1][i+1].visible==0)
        {
            ball[j][i].edge=1;
            ball[j-1][i+1].edge=2;
        }
    }

        if(i==0 && ball[j][i].visible==0 )
        {
            ball[j][i].edge=2;
        }
}}

void find_place()
{
    for(i=BOARD_HEIGHT-1;i>=0;i--)
    {for(j=0;j<BOARD_WIDTH;j++)
    {
        if(ball[j][i].edge==2)
        {
            d=(A*ball[j][i].x_position-ball[j][i].y_position+B)/sqrt(A*A+1);
            if(d<0)
                d=-d;

            if(d<=BALL_RADIUS)
            {
                shoot_to_i=i;
                shoot_to_j=j;
                break;
            }
        }
    }
    if (shoot_to_i>=0)
        break;
    }
}

void ball_disap1(int i, int j)
{
    if(ball[j][i+1].visible==1 && ball[j][i].colour==ball[j][i+1].colour && ball[j][i+1].edge!=3)
        {
            ball[j][i].edge=3;//edge=3 -> if there are at least 4 balls with edge 3 they will disappear
            ball[j][i+1].edge=3;
            disap1++;
            ball_disap1(i+1, j);
        }

        if(j!=16 && ball[j+1][i].visible==1 && ball[j][i].colour==ball[j+1][i].colour && ball[j+1][i].edge!=3)
        {
            ball[j][i].edge=3;
            ball[j+1][i].edge=3;
            disap1++;
            ball_disap1(i, j+1);
        }

        if(j!=0 && ball[j-1][i].visible==1 && ball[j][i].colour==ball[j-1][i].colour && ball[j-1][i].edge!=3)
        {
            ball[j][i].edge=3;
            ball[j-1][i].edge=3;
            disap1++;
            ball_disap1(i, j-1);
        }

        if(i!=0 && ball[j][i-1].visible==1 && ball[j][i].colour==ball[j][i-1].colour && ball[j][i-1].edge!=3)
        {
            ball[j][i].edge=3;
            ball[j][i-1].edge=3;
            disap1++;
            ball_disap1(i-1, j);
        }

        if(i!=0 && i%2==1 && j!=16 && ball[j+1][i-1].visible==1 && ball[j][i].colour==ball[j+1][i-1].colour && ball[j+1][i-1].edge!=3)
        {
            ball[j][i].edge=3;
            ball[j+1][i-1].edge=3;
            disap1++;
            ball_disap1(i-1, j+1);
        }

        if(i%2==1 && j!=16 && ball[j+1][i+1].visible==1 && ball[j][i].colour==ball[j+1][i+1].colour && ball[j+1][i+1].edge!=3)
        {
            ball[j][i].edge=3;
            ball[j+1][i+1].edge=3;
            disap1++;
            ball_disap1(i+1, j+1);
        }

        if(i!=0 && j!=0 && i%2==0 && ball[j-1][i-1].visible==1 && ball[j][i].colour==ball[j-1][i-1].colour && ball[j-1][i-1].edge!=3)
        {
            ball[j][i].edge=3;
            ball[j-1][i-1].edge=3;
            disap1++;
            ball_disap1(i-1, j-1);
        }

        if(i!=16 && j!=0 && i%2==0 && ball[j-1][i+1].visible==1 && ball[j][i].colour==ball[j-1][i+1].colour && ball[j-1][i+1].edge!=3)
        {
            ball[j][i].edge=3;
            ball[j-1][i+1].edge=3;
            disap1++;
            ball_disap1(i+1, j-1);
        }
}

void ball_disap2 (int i, int j, int edge)
{
    if(ball[j][i+1].visible==1 && ball[j][i+1].edge!=edge)
        {
            ball[j][i+1].edge=edge;
            if(edge==4)
            disap2++;
            ball_disap2(i+1, j, edge);
        }

        if(j!=16 && ball[j+1][i].visible==1 && ball[j+1][i].edge!=edge)
        {
            ball[j+1][i].edge=edge;
            if(edge==4)
            disap2++;
            ball_disap2(i, j+1, edge);
        }

        if(j!=0 && ball[j-1][i].visible==1 && ball[j-1][i].edge!=edge)
        {
            ball[j-1][i].edge=edge;
            if(edge==4)
            disap2++;
            ball_disap2(i, j-1, edge);
        }

        if(i!=0 && ball[j][i-1].visible==1 && ball[j][i-1].edge!=edge)
        {
            ball[j][i-1].edge=edge;
            if(edge==4)
            disap2++;
            ball_disap2(i-1, j, edge);
        }

        if(i!=0 && i%2==1 && j!=16 && ball[j+1][i-1].visible==1 && ball[j+1][i-1].edge!=edge)
        {
            ball[j+1][i-1].edge=edge;
            if(edge==4)
            disap2++;
            ball_disap2(i-1, j+1, edge);
        }

        if(i%2==1 && j!=16 && ball[j+1][i+1].visible==1 && ball[j+1][i+1].edge!=edge)
        {
            ball[j+1][i+1].edge=edge;
            if(edge==4)
            disap2++;
            ball_disap2(i+1, j+1, edge);
        }

        if(i!=0 && j!=0 && i%2==0 && ball[j-1][i-1].visible==1 && ball[j-1][i-1].edge!=edge)
        {
            ball[j-1][i-1].edge=edge;
            if(edge==4)
            disap2++;
            ball_disap2(i-1, j-1, edge);
        }

        if(i!=16 && j!=0 && i%2==0 && ball[j-1][i+1].visible==1 && ball[j-1][i+1].edge!=edge)
        {
            ball[j-1][i+1].edge=edge;
            if(edge==4)
            disap2++;
            ball_disap2(i+1, j-1, edge);
        }
}

void new_level()
{
    create_board();
    level_number++;//level number is the number of the level, level_numbers is the amount of numbers in the level_number
    itoa(level_number, level_char, 10);
    gtk_label_set_text(GTK_LABEL(level_label2), level_char);
}

void new_game()
{
    data_error=0;//if the game is started after an unsuccessful reading the data
    score=0;
    level_number=1;
    gtk_label_set_text(GTK_LABEL(score_label2), "0");
    gtk_label_set_text(GTK_LABEL(level_label2), "1");
    create_board();
    if(end_window_shown)
    {
        end_window_shown=0;
        gtk_widget_hide(end_window);
    }
    gtk_widget_set_sensitive(new_game_button, TRUE);
    gtk_widget_set_sensitive(exit_button, TRUE);
    gtk_widget_set_sensitive(save_b, TRUE);
    gtk_widget_set_sensitive(save_t, TRUE);
    gtk_widget_set_sensitive(read_b, TRUE);
    gtk_widget_set_sensitive(read_t, TRUE);
}

void game_over()
{
    gtk_label_set_text(GTK_LABEL(end_label1), "Game over! Your score: ");
    gtk_label_set_text(GTK_LABEL(end_label2), score_char);
    gtk_widget_show_all(end_window);
    end_window_shown=1;
    gtk_widget_set_sensitive(new_game_button, FALSE);
    gtk_widget_set_sensitive(exit_button, FALSE);
    gtk_widget_set_sensitive(save_b, FALSE);
    gtk_widget_set_sensitive(save_t, FALSE);
    gtk_widget_set_sensitive(read_b, FALSE);
    gtk_widget_set_sensitive(read_t, FALSE);
}
void create_end_window()
{
    end_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(end_window), GTK_WIN_POS_CENTER);
    gtk_widget_set_size_request(end_window, SMALL_WINDOW_WIDTH, SMALL_WINDOW_WIDTH/2);
    gtk_window_set_resizable (GTK_WINDOW(end_window), FALSE);//changing the size of the window is impossible
    gtk_window_set_title(GTK_WINDOW(end_window), "Bust a move");

    end_container = gtk_fixed_new();
    gtk_container_add(GTK_CONTAINER(end_window),end_container);
    end_label1= gtk_label_new(NULL);
    gtk_fixed_put(GTK_FIXED(end_container), end_label1, 30, 10);
    end_label2= gtk_label_new(NULL);
    gtk_fixed_put(GTK_FIXED(end_container), end_label2, 90, 30);

    end_button1 = gtk_button_new_with_label("New board");
    gtk_widget_set_size_request(end_button1, 80, 30);
    gtk_fixed_put(GTK_FIXED(end_container), end_button1, 15, 60);

    end_button2 = gtk_button_new_with_label("Exit");
    gtk_widget_set_size_request(end_button2, 80, 30);
    gtk_fixed_put(GTK_FIXED(end_container), end_button2, 105, 60);
}

void save_binary()
{
    if((file=fopen("binary_save_read.txt", "wb"))==NULL)
    {
        printf("Opening file - error");
        gtk_main_quit();
    }

        fwrite(&BALL_COL, sizeof(int), 1, file);
        fwrite(&new_ball_colour, sizeof(int), 1, file);
        fwrite(&score, sizeof(int), 1, file);
        fwrite(&level_number, sizeof(int), 1, file);
        fwrite(&balls_down, sizeof(int), 1, file);

    for(i=0;i<BOARD_HEIGHT;i++)
        {
            for(j=0;j<BOARD_WIDTH;j++)
            fwrite(&(ball[j][i].colour), sizeof(int), 1, file);
        }//saving the board of balls

fclose(file);
}

void save_text()
{
    if((file=fopen("text_save_read.txt", "w"))==NULL)
    {
        printf("Opening file - error");
        gtk_main_quit();
    }
    fprintf(file, "%d %d %d %d %d\n", BALL_COL, new_ball_colour, score, level_number, balls_down);//number of colours, shooting ball colour, score, level, if balls should go down
    for(i=0;i<BOARD_HEIGHT;i++)
    {
        for(j=0;j<BOARD_WIDTH;j++)
            fprintf(file, "%d ", ball[j][i].colour);
        fprintf(file, "\n");
    }//saving the boar of balls
    fclose(file);

}

void read_binary()
{
    if((file=fopen("binary_save_read.txt", "rb"))==NULL)
    {
        printf("Opening file - error");
        read_error();
    }
    if(fread(&BALL_COL, sizeof(int), 1, file)==0) read_error();
    if(fread(&new_ball_colour, sizeof(int), 1, file)==0) read_error();
    if(fread(&score, sizeof(int), 1, file)==0) read_error();
    if(fread(&level_number, sizeof(int), 1, file)==0) read_error();
    if(fread(&balls_down, sizeof(int), 1, file)==0) read_error();
    if(BALL_COL>6 || BALL_COL<1 || new_ball_colour>BALL_COL || new_ball_colour<1 ||score<0 || level_number<1 || balls_down<0) read_error();

    balls_down_read=balls_down;
    max_balls_place=0;
    shot_balls=0;
    max_balls=14-balls_down;
    balls_amount=0;

    if(!data_error)
    for(i=0;i<BOARD_HEIGHT;i++)
    {
        for(j=0;j<BOARD_WIDTH;j++)
            {
            if(fread(&ball[j][i].colour, sizeof(int), 1, file)==0 || ball[j][i].colour>BALL_COL || ball[j][i].colour<0)
                read_error();
            else
                {
                    if(ball[j][i].colour==0)
                        ball[j][i].visible=0;
                    else
                        {
                            ball[j][i].visible=1;
                            balls_amount++;
                            if(i>max_balls_place)
                                max_balls_place=i;
                        }
                ball[j][i].y_position=20+i*24+24*balls_down;
                ball[j][i].edge=0;
                }
            }
    }

check_read();
fclose(file);
}

void read_text()
{
    if((file=fopen("text_save_read.txt", "r"))==NULL)
    {
        printf("Opening file - error");
        read_error();
    }

    if(fscanf(file, "%d %d %d %d %d", &BALL_COL, &new_ball_colour, &score, &level_number, &balls_down)!=5 || BALL_COL>6 || BALL_COL<1 || new_ball_colour>BALL_COL || new_ball_colour<1 ||score<0 || level_number<1 || balls_down<0)
        read_error();

    balls_down_read=balls_down;
    max_balls_place=0;
    shot_balls=0;
    max_balls=14-balls_down;
    balls_amount=0;

    if(!data_error)
    for(i=0;i<BOARD_HEIGHT;i++)
    {
        for(j=0;j<BOARD_WIDTH;j++)
            {
            if(fscanf(file, "%d", &ball[j][i].colour)==0 || ball[j][i].colour>BALL_COL || ball[j][i].colour<0)
                read_error();
            else
                {
                    if(ball[j][i].colour==0)
                        ball[j][i].visible=0;
                    else
                        {
                            ball[j][i].visible=1;
                            balls_amount++;
                            if(i>max_balls_place)
                                max_balls_place=i;
                        }
                ball[j][i].y_position=20+i*24+24*balls_down;
                ball[j][i].edge=0;
                }
            }
    }
check_read();
fclose(file);
}

void read_error()
{
    fclose(file);
    data_error=1;
    gtk_label_set_text(GTK_LABEL(end_label1), "Wrong form of the data.");
    gtk_label_set_text(GTK_LABEL(end_label2), "");
    gtk_widget_show_all(end_window);
    end_window_shown=1;
    gtk_widget_set_sensitive(new_game_button, FALSE);
    gtk_widget_set_sensitive(exit_button, FALSE);
    gtk_widget_set_sensitive(save_b, FALSE);
    gtk_widget_set_sensitive(save_t, FALSE);
    gtk_widget_set_sensitive(read_b, FALSE);
    gtk_widget_set_sensitive(read_t, FALSE);
    disap2=0;

    for(i=0;i<BOARD_HEIGHT;i++)
    for(j=0;j<BOARD_WIDTH;j++)
            ball[j][i].visible=0;
}

void check_read()
{
    if(!data_error)
    {
        check_edge(0, 0);
        for(i=0;i<BOARD_HEIGHT;i++)
        for(j=0;j<BOARD_WIDTH;j++)
            {
            //edge=4 when balls should disappear and they are worth 2 points
            //edge=5 when balls should not disappear
                if(ball[j][i].edge!=4 && ball[j][i].edge!=5  && ball[j][i].visible==1)
                    {if(i==0)
                        {ball[j][i].edge=5;
                         ball_disap2(i, j, 5);}
                    else
                        {ball[j][i].edge=4;
                        disap2++;
                        ball_disap2(i, j, 4);}
                    }
            }//checking if the ball's layout is correct
        }
    if(disap2)
        read_error();
    else
        check_edge(0,0);

    if(!data_error)
    {
    itoa(score, score_char, 10);
    gtk_label_set_text(GTK_LABEL(score_label2), score_char);//changing the label
    itoa(level_number, level_char, 10);
    gtk_label_set_text(GTK_LABEL(level_label2), level_char);
    }
}
