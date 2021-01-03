#include <gtkmm.h>

void change_background_color(Gtk::Button &button) {
    button.get_style_context().clear();
    Glib::RefPtr<Gtk::CssProvider> provider = Gtk::CssProvider::create();
    provider->load_from_data("button {background-image: image(green)}");
    button.get_style_context()->add_provider(provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
}

int main(int argc, char *argv[])
{
    auto app =
            Gtk::Application::create(argc, argv,
                                     "org.gtkmm.examples.base");

    /*Gtk::Button button;

    Gdk::Color color("red");
    button.set_label("button");
    Glib::RefPtr<Gtk::CssProvider> provider = Gtk::CssProvider::create();
    provider->load_from_data("button {background-image: image(red)}");

    button.get_style_context().clear();
    button.get_style_context()->add_provider(provider, GTK_STYLE_PROVIDER_PRIORITY_USER);

    button.signal_clicked().connect(sigc::bind<Gtk::Button &>(sigc::ptr_fun(&change_background_color), button));


    window.add(button);
    button.show();*/

    Gtk::Window window;
    window.set_default_size(400, 400);

    Gtk::Button button1("button1"), button2("button2"), button3("button3"), button4("button4");

    Gtk::Grid grid;
    grid.set_row_homogeneous(true);
    grid.set_column_homogeneous(true);
    grid.attach(button1, 0, 0);
    grid.attach(button2, 0, 1);
    grid.attach(button3, 1, 0);
    grid.attach(button4, 1, 1);
    window.add(grid);
    grid.show_all();
    return app->run(window);
}