#pragma once

#include <thread>
#include <functional>
#include <mutex>
#include <vector>
#include "../Core/Types.h"
#include "../Util/FileUtils.h"


namespace ServerCore
{


class ThreadManager
{
public:
    ThreadManager();
    ~ThreadManager();
    void Test();
    /* ������ �����̳�(vector<thread>	threads)�� �����带 �߰��ϰ�,
    ���ÿ� �ش� �������� �Լ���ü callback�� �����Ѵ�*/
    void Launch(std::function<void(void)> callback);
    /* ������ �����̳�(vector<thread>	threads)�� �ִ� ��� ������鿡 ���Ͽ� join�� �����ϰ�,
    join�� ��ȯ�ϸ� ������ �����̳ʸ� ����*/
    void JoinAll();
    // ������ ID�� �ο��ϰ� ���������� TLS�� �ش� ������ ID�� ������ ��
    static void InitTLS();
    static void DestroyTLS();

    static void DoGlobalQueueWork();
    static void CheckJobTimer();

private:
    std::mutex mtxLock;
    std::vector<std::thread> threads;
};
}