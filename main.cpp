#include "passa_tempo.hpp"
#include <bits/stdc++.h>
#include <pthread.h>

#define endl '\n'

using namespace std;

struct Room {
    int id;
    int stay_time;
};

struct ThreadData {
    int id;
    int wait_time;
    int num_rooms;
    vector<Room> rooms;
};

// "o número máximo de salas será 10 e o número máximo de threads será o triplo do número de salas;"
constexpr int THREAD_POOL_SIZE = 10 * 3;

array<pthread_t, THREAD_POOL_SIZE> thread_pool;

array<pthread_mutex_t, THREAD_POOL_SIZE> mutex_room_waiter;
array<pthread_mutex_t, THREAD_POOL_SIZE> mutex_room_current;

array<pthread_cond_t, THREAD_POOL_SIZE> cond_counter;

array<int, THREAD_POOL_SIZE> room_waiters;
array<int, THREAD_POOL_SIZE> room_current;

// Pré-condição: não estou em uma sala
void enter(int room_id) {
    pthread_mutex_lock(&mutex_room_waiter.at(room_id));

    auto &waiters = room_waiters.at(room_id);

    waiters++;

    while (waiters < 3 || waiters > 3) {
        pthread_cond_wait(&cond_counter.at(room_id), &mutex_room_waiter.at(room_id));
    }

    // NOTE remover
    // cout << thread_id << " " << room_id << " " << waiters << " W" << endl;

    pthread_cond_broadcast(&cond_counter.at(room_id));

    pthread_mutex_lock(&mutex_room_current.at(room_id));

    auto &current = room_current.at(room_id);

    if (current == 0) {
        current = 3;
    }

    pthread_mutex_unlock(&mutex_room_current.at(room_id));

    pthread_mutex_unlock(&mutex_room_waiter.at(room_id));
}

// Pré-condição: estou em uma sala
void leave(int room_id) {
    pthread_mutex_lock(&mutex_room_current.at(room_id));
    auto &current = room_current.at(room_id);

    current--;

    // NOTE remover
    // cout << thread_id << " " << room_id << " " << current << " C" << endl;

    if (current == 0) {
        pthread_mutex_lock(&mutex_room_waiter.at(room_id));

        auto &waiters = room_waiters.at(room_id);

        waiters -= 3;

        pthread_mutex_unlock(&mutex_room_waiter.at(room_id));

        pthread_cond_broadcast(&cond_counter.at(room_id));
    }

    pthread_mutex_unlock(&mutex_room_current.at(room_id));
}

void *thread_function(void *data) {
    auto *thread_data = static_cast<ThreadData *>(data);
    auto &num_rooms = thread_data->num_rooms;
    auto &rooms = thread_data->rooms;

    passa_tempo(thread_data->id, 0, thread_data->wait_time);

    for (int i = 0; i < num_rooms; ++i) {
        enter(rooms[i].id);
        passa_tempo(thread_data->id, rooms[i].id, rooms[i].stay_time);
        leave(rooms[i].id);
    }

    // TODO atualizar ou remover esse comentário
    // enquanto existe próxima sala S' que pode ser retirada do trajeto
    //        quando as regras o permitirem (forma trio, sala está vazia),
    //              entra na próxima sala;
    //              libera a posição anterior (se existir);
    //              executa a função passa_tempo com o tempo associado à posição S';
    //         fim quando
    // fim enquanto
    // libera a posição da última sala.

    return nullptr;
}

int main() {
    int num_all_rooms = 0;
    int num_all_threads = 0;

    cin >> num_all_rooms >> num_all_threads;

    for (int i = 0; i < num_all_threads; ++i) {
        pthread_mutex_init(&mutex_room_waiter.at(i), nullptr);
        pthread_mutex_init(&mutex_room_current.at(i), nullptr);
        pthread_cond_init(&cond_counter.at(i), nullptr);
    }

    vector<ThreadData> all_thread_data(num_all_threads);

    for (int i = 0; i < num_all_threads; ++i) {
        auto &num_rooms = all_thread_data[i].num_rooms;
        auto &rooms = all_thread_data[i].rooms;

        cin >> all_thread_data[i].id >> all_thread_data[i].wait_time >> num_rooms;

        rooms.resize(num_rooms);

        for (int j = 0; j < num_rooms; j++) {
            cin >> rooms[j].id >> rooms[j].stay_time;
        }
    }

    for (int i = 0; i < num_all_threads; ++i) {
        pthread_create(&thread_pool.at(i), nullptr, thread_function, (void *)&all_thread_data[i]);
    }

    for (int i = 0; i < num_all_threads; ++i) {
        pthread_join(thread_pool.at(i), nullptr);
    }

    return 0;
}
