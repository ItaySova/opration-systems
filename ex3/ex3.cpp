// author: Itay Sova id: (removed)
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <deque>
#include <semaphore.h>
#include <chrono>
using namespace std;

// a unit of data taken from conf.txt
struct producer_data{
    int producers_index;
    int number_of_news;
    int queue_size;
};

//classes:
class Bounded_Buffer {
public:
    int size;
    mutex mutex_;
    deque<string> buffer;
    sem_t notEmpty{};
    sem_t notFull{};
    // regular constructor
    Bounded_Buffer(int i) {
        size = i;
        // notempty - the number of full cells
        sem_init(&this->notEmpty, 0, 0);
        // notfull - the number of empty cells
        sem_init(&this->notFull, 0, size);
    }
    // copy constructor:
    Bounded_Buffer(const Bounded_Buffer& other){
        this->size = other.size;
        this->buffer = other.buffer;
        sem_init(&this->notEmpty, 0, 0);
        sem_init(&this->notFull, 0, this->size);
    }
    // destructor
    ~Bounded_Buffer(){
        sem_destroy(&this->notEmpty);
        sem_destroy(&this->notFull);
    }
    void insert (string s){
        sem_wait(&this->notFull);
        this->mutex_.lock();
        this->buffer.push_back(s);
        this->mutex_.unlock();
        sem_post(&this->notEmpty);
    }
    string inner_pop(){
        string news = this->buffer.front();
        this->buffer.pop_front();
        return news;
    }
    string remove(){
        sem_wait(&this->notEmpty);
        this->mutex_.lock();
        string news = this->inner_pop();
        this->mutex_.unlock();
        sem_post(&this->notFull);
        return news;
    }
};

class Unbounded_Buffer{
public:
    mutex mutex_;
    sem_t notEmpty{};
    deque<string> buffer;
    Unbounded_Buffer() {
        sem_init(&this->notEmpty, 0, 0);
    }
    Unbounded_Buffer(const Unbounded_Buffer& other){
        this->buffer = other.buffer;
        sem_init(&this->notEmpty, 0, 0);
    }
    ~Unbounded_Buffer(){
        sem_destroy(&this->notEmpty);
    }
    void insert (string s){
        // change back to void insert (char* s)
        // lock and then push, unlock and notify the semaphore
        this->mutex_.lock();
        this->buffer.push_back(s);
        this->mutex_.unlock();
        sem_post(&this->notEmpty);
    }
    string remove(){ // todo - continue from here
        // change to returning char*
        //cout << "buffer current size: " << this->buffer.size() << "\n";
        sem_wait(&this->notEmpty);
        //cout << "remove past wait for not empty\n";
        this->mutex_.lock();
        //cout << "remove past for mutex\n";
        string news = this->buffer.front();
        this->buffer.pop_front();
        //cout << "remove inner pop()\n";
        this->mutex_.unlock();
        //cout << "remove past post mutex\n";
        return news;
    }
};
// global variables:
vector<Bounded_Buffer> producers_to_dispatcher_queues;
Unbounded_Buffer dispatcher_to_sports_coEditor;
Unbounded_Buffer dispatcher_to_news_coEditor;
Unbounded_Buffer dispatcher_to_weather_coEditor;
Bounded_Buffer *coEditors_to_sm_buffer;
// global variable for storing the data from the conf.txt
vector<producer_data> data_from_conf;
int coEditor_queue_size;

// functions declaration:

// functions implementation:

// function for taking the data from the conf.txt and storing it in the vector - fix for the input after done
// add tests if there will be still time
void from_conf_to_vector(string file_path){
    string line;
    // full version
    //ifstream conf_file("conf.txt");
    ifstream conf_file(file_path);
    // one prod version
    // two prods version
    int producer_unit_index = 1;
    producer_data p_temp;
    while(getline(conf_file,line)){
        //cout << "\nline is: " << line << " line length: " << line.length() << "\n";
        // in case the configurations file has cr, lf end of lines instead of just lf
        if(line == "\r" || line == "\n"){
            cout << "\ncatched the whitespace\n";
            producer_unit_index = 1;
            data_from_conf.push_back(p_temp);
            // after pushing to the vector - set the temp variables to 0 for tests
            p_temp.producers_index = 0;
            p_temp.queue_size = 0;
            p_temp.number_of_news = 0;
            continue;
        }
        if(line.length() == 0){
            producer_unit_index = 1;
            data_from_conf.push_back(p_temp);
            // after pushing to the vector - set the temp variables to 0 for tests
            p_temp.producers_index = 0;
            p_temp.queue_size = 0;
            p_temp.number_of_news = 0;
            continue;
        }
        if (line.length() != 0){
            int line_int = stoi (line,nullptr,0);
            switch (producer_unit_index) {
                case 1:
                    p_temp.producers_index = int(line_int);
                    producer_unit_index++;
                    break;
                case 2:
                    p_temp.number_of_news = int(line_int);
                    producer_unit_index++;
                    break;
                case 3:
                    p_temp.queue_size = int(line_int);
                    producer_unit_index++;
                    break;
            }
        }
    }
    conf_file.close();
    if (p_temp.producers_index != 0 && p_temp.queue_size == 0 && p_temp.number_of_news == 0){
        coEditor_queue_size = p_temp.producers_index;
    }
}

// print function for the producer_data vector - initial version
void print_vector(vector<producer_data> data){
    while (!data.empty()){
        cout << "producers data: \n";
        cout << "producers_index: " << data.back().producers_index << "\n";
        cout << "number_of_news: " << data.back().number_of_news << "\n";
        cout << "queue_size: " << data.back().queue_size << "\n";
        data.pop_back();
    }
}

// print function for the producer_data vector - second version
void print_vector_corrected(vector<producer_data> data){
    int vector_index = 0;
    int vector_size = data.size();
    //cout << "type of begin: " << typeid(data.begin()).name() << endl;
    for (;vector_index < vector_size; vector_index++){
        cout << "producer index: "<< data[vector_index].producers_index << "\n";
        cout << "number_of_news: " << data[vector_index].number_of_news << "\n";
        cout << "queue_size: " << data[vector_index].queue_size << "\n";
    }
}

// print function for the strings from Bounded_Buffer vector
void print_vector_corrected_string(Bounded_Buffer data){
    int vector_index = 0;
    int vector_size = data.buffer.size();
    for (;vector_index < vector_size; vector_index++){
        cout << data.remove() << "\n";
    }
}
void print_string_vector(deque<string> data){
    int vector_index = 0;
    int vector_size = data.size();
    for (;vector_index < vector_size; vector_index++){
        cout << data.front() << "\n";
        data.pop_front();
    }
}


void producer(int prod_index){
    //cout << "\nthis is a producer's "<< prod_index<<" work\n";
    // each time prod create news the format will be "producer <prod_index> <news_type> <news_index>"
    // take the number of products and index from the struct defined to store each producer's
    // details.
    int number_of_products = data_from_conf[prod_index].number_of_news;
    int producer_id = data_from_conf[prod_index].producers_index;
    // init the indexes for the different types of news.
    int news_index = 0, sport_index = 0, weather_index = 0;
    for(int i = 0; i < number_of_products; i++){
        // create news
        string news;
        int news_type_index = prod_index + i;
        // note - if needed a single type of news per producer
        // change switch to switch (producer_id % 3)
        // real time -  switch (news_type_index % 3)
        switch (news_type_index % 3) {
            case 0:
                news = "Producer " + to_string(producer_id) + " sports " + to_string(sport_index);
                sport_index += 1;
                break;
            case 1:
                news = "Producer " + to_string(producer_id) + " news " + to_string(news_index);
                news_index += 1;
                break;
            case 2:
                news = "Producer " + to_string(producer_id) + " weather " + to_string(weather_index);
                weather_index += 1;
                break;
        }
        // push news to the appropriate buffer
        //cout << "\nattempt to push to buffer "<< prod_index << "\n";
        producers_to_dispatcher_queues[prod_index].insert(news);
        //cout << "pushed succeccfull\n";
    }
    // after finishing all his news products add another "done" string for later functions
    //cout << "\ndone with the list of producer " << prod_index <<"\n";
    producers_to_dispatcher_queues[prod_index].insert("done");
    //cout << "\ndone with producer " << prod_index << "\n";
}

void dispatcher(){
    //cout << "will be dispatcher function fo thread\n";
    // number of producers is taken from the size of vector storing the producers details
    int num_of_producers = data_from_conf.size();
    bool are_all_producers_done = false;
    // start running on the buffers and classify the news to the different co editors
    while(!are_all_producers_done){
        //cout << "start going in circle\n";
        int prod_index = 0;
        // each iteration init the num_of_producers_finished and if the num = the num of producers we finish the loop
        int num_of_producers_finished = 0;
        // a loop that go over all the buffers, once each
        for (; prod_index < num_of_producers; prod_index++){
            //cout << "trying to remove from producer " << prod_index <<  " in inner circle\n";
            string current_news = producers_to_dispatcher_queues[prod_index].remove();
            //cout << "remove successfully from producer " << prod_index <<  " in inner circle\n";
            // check if the producer is done and if so update the index of done prods and
            // continue to the next iteration of the loop
            if (current_news == "done"){
                num_of_producers_finished += 1;
                //cout << "\nproducer "<< prod_index << " is done\n";
                //cout << "\nnum of producers done: " << num_of_producers_finished;
                producers_to_dispatcher_queues[prod_index].insert("done");
                // check if all the producers are done and if so break:
                if (num_of_producers_finished == num_of_producers){
                    are_all_producers_done = true;
                    //cout << "\nall producers are done\n";
                    break;
                }
                continue;
            }
            //cout << current_news << "\n";
            size_t is_sports = current_news.find("sports");
            size_t is_news = current_news.find("news");
            size_t is_weather = current_news.find("weather");
            if(is_sports != string::npos){
                // cout <<"\nSPORTS Inserted to the S dispatcher queue\n";
                dispatcher_to_sports_coEditor.insert(current_news);
            }
            if (is_news != string::npos){
                // cout <<"\nNEWS Inserted to the N dispatcher queue\n";
                dispatcher_to_news_coEditor.insert(current_news);
            }
            if (is_weather != string::npos){
                // cout <<"\nWEATHER Inserted to the W dispatcher queue\n";
                dispatcher_to_weather_coEditor.insert(current_news);
            }
        }
    }
    // appending a finish massage:
    dispatcher_to_news_coEditor.insert("done");
    dispatcher_to_sports_coEditor.insert("done");
    dispatcher_to_weather_coEditor.insert("done");
    // cout << "done with dispatcher\n";
}

void coEditor(int num_of_buffer){
    Unbounded_Buffer* appropriate_buffer;
    switch (num_of_buffer) {
        case 1:
            appropriate_buffer = &dispatcher_to_sports_coEditor;
            break;
        case 2:
            appropriate_buffer = &dispatcher_to_news_coEditor;
            break;
        case 3:
            appropriate_buffer = &dispatcher_to_weather_coEditor;
            break;
    }
    bool done_with_dispatcher_buffer = false;
    while (!done_with_dispatcher_buffer){
        string last_news_from_dispatch = appropriate_buffer->remove();
        if( last_news_from_dispatch == "done"){
            done_with_dispatcher_buffer = true;
            coEditors_to_sm_buffer->insert("done");
        } else{
            // sleep for 0.1 sec (aka "edit the news") before inserting to the queue
            this_thread::sleep_for (chrono::milliseconds (100));
            coEditors_to_sm_buffer->insert(last_news_from_dispatch);
        }
    }
    // cout << "\ndone with co editor\n";
}

void screenManager(){
    //cout << "\nscreen managers work:\n";
    int num_of_editors_finished = 0;
    while (num_of_editors_finished < 3){
        // going over the appropriate buffer, removing the first element and if it is not "done"
        // print the news
        string last_news_from_coEditors = coEditors_to_sm_buffer->remove();
        // checking if the last massage is "done" and count the number of "done" it gets.
        if (last_news_from_coEditors == "done"){
            num_of_editors_finished += 1;
        } else{
            cout << "\n"<<last_news_from_coEditors;
        }
    }
    // finished massage
    cout << "\nDONE\n";
}

int main(int argc, char *argv[]){
    if (argc < 2){
        cout << "configurations file path is missing\n";
        return -1;
    }
    string file_path = argv[1];
    from_conf_to_vector(file_path);
    int num_of_producers = data_from_conf.size();
    //cout << "number of producers: " << num_of_producers << "\n";
    //cout << "coEditor queue size: " << coEditor_queue_size << "\n";
    Bounded_Buffer editors_to_screen_manager(coEditor_queue_size);
    coEditors_to_sm_buffer = &editors_to_screen_manager;
    for(int i = 0; i<num_of_producers; i++){
        int size_for_q = data_from_conf[i].queue_size;
        // new buffer init and appending into the global buffers vector
        Bounded_Buffer local_buf(size_for_q);
        producers_to_dispatcher_queues.push_back(local_buf);
    }
    // init a vector of threads:
    vector<thread> threads;
    //cout << "creating threads: \n";
    for (int i = 0; i< num_of_producers; i++){
        // create a thread and push it to the vector
        threads.emplace_back(producer, i);
    }
    threads.emplace_back(dispatcher);
    threads.emplace_back(coEditor, 1);
    threads.emplace_back(coEditor, 2);
    threads.emplace_back(coEditor, 3);
    threads.emplace_back(screenManager);
    int num_of_threads = threads.size();
    for (int i= 0; i< num_of_threads; i++){
        threads[i].join();
    }
    return 0;
}

// The compilation command is:
//
//    g++ ex3.cpp -o ex3.out -std=c++11 -pthread
//
// The run command the file need to recieve is:
//
//     ./ex3.out <configuration file name>
// for more info - go to the readme file
