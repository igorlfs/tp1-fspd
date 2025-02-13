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
    int prev_room_id;
    vector<Room> rooms;
};

// "o número máximo de salas será 10 e o número máximo de threads será o triplo do número de salas;"
constexpr int THREAD_POOL_SIZE = 10 * 3;

array<pthread_t, THREAD_POOL_SIZE> thread_pool;

array<pthread_mutex_t, THREAD_POOL_SIZE> mutex_room_waiting;
array<pthread_mutex_t, THREAD_POOL_SIZE> mutex_room_full;

array<pthread_cond_t, THREAD_POOL_SIZE> cond_min_waiting;
array<pthread_cond_t, THREAD_POOL_SIZE> cond_max_waiting;
array<pthread_cond_t, THREAD_POOL_SIZE> cond_full;

array<int, THREAD_POOL_SIZE> room_waiting;
array<bool, THREAD_POOL_SIZE> room_full;

// Pré-condição: não estou em uma sala
void enter(int room_id) {
    pthread_mutex_lock(&mutex_room_full.at(room_id));

    // Esperar enquanto a sala estiver ocupada
    while (room_full.at(room_id)) {
        pthread_cond_wait(&cond_full.at(room_id), &mutex_room_full.at(room_id));
    }

    pthread_mutex_unlock(&mutex_room_full.at(room_id));

    pthread_mutex_lock(&mutex_room_waiting.at(room_id));

    auto &waiting = room_waiting.at(room_id);

    // Em princípio, é possível que 4 threads passem pelo mutex para checar se a sala está ocupada
    // Nesse caso, poderia acontecer de essas 4 threads rodarem simultaneamente, o que não é desejado
    // Confirmei que isso realmente pode acontecer, com um exemplo de entrada
    // que tinha apenas uma sala, mas muitas threads
    while (waiting == 3) {
        pthread_cond_wait(&cond_max_waiting.at(room_id), &mutex_room_waiting.at(room_id));
    }

    waiting++;

    // As duas primeiras threads que "passarem", deverão ficar esperando pela terceira
    while (waiting < 3) {
        pthread_cond_wait(&cond_min_waiting.at(room_id), &mutex_room_waiting.at(room_id));
    }

    // Se chegamos até aqui, a sala agora está cheia
    pthread_mutex_lock(&mutex_room_full.at(room_id));
    room_full.at(room_id) = true;
    pthread_mutex_unlock(&mutex_room_full.at(room_id));

    pthread_cond_broadcast(&cond_min_waiting.at(room_id));

    pthread_mutex_unlock(&mutex_room_waiting.at(room_id));
}

// Pré-condição: estou em uma sala
void leave(int room_id) {
    pthread_mutex_lock(&mutex_room_waiting.at(room_id));

    auto &waiting = room_waiting.at(room_id);

    // É possível decrementar a quantidade de threads "atualmente" na sala
    // até chegar em zero, para esvaziá-la
    waiting--;

    if (waiting == 0) {

        // Sala liberada
        pthread_mutex_lock(&mutex_room_full.at(room_id));
        room_full.at(room_id) = false;
        pthread_mutex_unlock(&mutex_room_full.at(room_id));

        // Desperta quem estava travado por causa da sala estar cheia
        pthread_cond_broadcast(&cond_full.at(room_id));

        // Também desperta quem estava travado porque só passam 3 threads por vez
        pthread_cond_broadcast(&cond_max_waiting.at(room_id));
    }

    pthread_mutex_unlock(&mutex_room_waiting.at(room_id));
}

void *thread_function(void *data) {
    auto *thread_data = static_cast<ThreadData *>(data);
    auto &num_rooms = thread_data->num_rooms;
    auto &rooms = thread_data->rooms;
    auto &prev_room_id = thread_data->prev_room_id;

    passa_tempo(thread_data->id, 0, thread_data->wait_time);

    for (int i = 0; i < num_rooms; ++i) {
        enter(rooms[i].id);
        if (prev_room_id != 0) {
            leave(prev_room_id);
        }
        passa_tempo(thread_data->id, rooms[i].id, rooms[i].stay_time);
        prev_room_id = rooms[i].id;
    }
    leave(prev_room_id);

    return nullptr;
}

int main() {
    int num_all_rooms = 0;
    int num_all_threads = 0;

    cin >> num_all_rooms >> num_all_threads;

    for (int i = 0; i < num_all_threads; ++i) {
        pthread_mutex_init(&mutex_room_waiting.at(i), nullptr);
        pthread_mutex_init(&mutex_room_full.at(i), nullptr);
        pthread_cond_init(&cond_min_waiting.at(i), nullptr);
        pthread_cond_init(&cond_max_waiting.at(i), nullptr);
        pthread_cond_init(&cond_full.at(i), nullptr);
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
