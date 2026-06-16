#include "ble_manager.h"

#include <stdio.h>
#include <string.h>

#include "nvs_flash.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
/* GAP - Generic Access Profile To część BLE odpowiedzialna za:
- nazwę urządzenia
- advertising
- łączenie
- rozłączanie
*/
#include "host/ble_gap.h" 
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "host/ble_uuid.h"
#include "host/ble_gatt.h"

// Aktualna liczba kroków udostępniana przez BLE.
// Jest aktualizowana z poziomu main.cpp po wykryciu kroku.
static uint32_t current_step_count = 0;

// Uchwyt charakterystyki FFF1.
// NimBLE przypisuje go podczas rejestracji GATT.
// Jest potrzebny później do wysyłania Notify.
static uint16_t step_count_handle = 0;

// Uchwyt aktualnego połączenia BLE.
// Dzięki niemu ESP32 wie, do którego telefonu wysłać Notify.
static uint16_t current_conn_handle = 0;

static bool device_connected = false;

// Funkcja wywoływana automatycznie, gdy telefon wykona READ
// na charakterystyce FFF1.
static int step_count_access_cb(
    uint16_t conn_handle,
    uint16_t attr_handle,
    struct ble_gatt_access_ctxt* ctxt,
    void* arg)
{
    char value[16];

    // Zamiana liczby kroków na tekst, np. 15 -> "15".
    snprintf(value, sizeof(value), "%lu", current_step_count);

    // Wpisanie tekstu do bufora odpowiedzi BLE.
    int result = os_mbuf_append(ctxt->om, value, strlen(value));

    if (result == 0)
    {
        return 0;
    }

    return BLE_ATT_ERR_INSUFFICIENT_RES;
}

// UUID naszej własnej usługi BLE.
// Telefon będzie widział ją jako usługę FFF0.
static const ble_uuid16_t step_service_uuid = BLE_UUID16_INIT(0xFFF0);

// UUID charakterystyki przechowującej liczbę kroków.
// Telefon będzie odczytywał dane z FFF1.
static const ble_uuid16_t step_count_uuid = BLE_UUID16_INIT(0xFFF1);

// GATT
// Charakterystyka FFF1 przechowująca liczbę kroków.
static const struct ble_gatt_chr_def step_characteristics[] = {
    {
        (const ble_uuid_t*)&step_count_uuid,
        step_count_access_cb,
        nullptr,
        nullptr,
        /* Telefon może:
        - odczytać wartość (READ)
        - otrzymywać automatyczne aktualizacje (NOTIFY)
        */
        BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
        0,
        &step_count_handle,
        nullptr
    },
    {
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        0,
        0,
        nullptr,
        nullptr
    }
};

// GATT
// Główna usługa BLE naszego krokomierza.
static const struct ble_gatt_svc_def gatt_services[] = {
    {
        BLE_GATT_SVC_TYPE_PRIMARY,
        (const ble_uuid_t*)&step_service_uuid,
        nullptr,
        step_characteristics
    },
    {
        0,
        nullptr,
        nullptr,
        nullptr
    }
};

static void start_advertising(void);

// Zadanie FreeRTOS, w którym działa stos NimBLE.
// Ta funkcja utrzymuje obsługę Bluetooth w tle.
static void ble_host_task(void* param)
{
    printf("[BLE_Manager] BLE host task started.\n");

    // Główna pętla obsługi NimBLE.
    // Bez tego BLE byłoby skonfigurowane, ale nie działałoby aktywnie.
    nimble_port_run();

    // Sprzątanie zasobów NimBLE po zakończeniu pracy.
    nimble_port_freertos_deinit();
}

// Funkcja wywoływana automatycznie, gdy NimBLE jest gotowe do pracy.
static void ble_on_sync(void)
{
    printf("[BLE_Manager] BLE synchronized.\n");

    // Po pełnym uruchomieniu BLE zaczynamy advertising,
    // czyli urządzenie staje się widoczne dla telefonu.
    start_advertising();
}

// Konstruktor
BLE_Manager::BLE_Manager()
{
}

// Aktualizuje liczbę kroków przechowywaną w module BLE.
// Ta wartość jest później odczytywana przez telefon lub wysyłana przez Notify.
void BLE_Manager::updateStepCount(uint32_t stepCount)
{
    current_step_count = stepCount;
}

void BLE_Manager::init()
{
    // Inicjalizacja pamięci NVS wymaganej przez BLE.
    esp_err_t result = nvs_flash_init();

    if (result != ESP_OK)
    {
        printf("[BLE_Manager] Blad NVS.\n");
        return;
    }

    printf("[BLE_Manager] NVS uruchomione poprawnie.\n");

    // Inicjalizacja stosu NimBLE.
    nimble_port_init();

    // Rejestracja funkcji wywoływanej po pełnym uruchomieniu BLE.
    ble_hs_cfg.sync_cb = ble_on_sync;

    // Inicjalizacja usług GAP i GATT.
    ble_svc_gap_init();
    ble_svc_gatt_init();

    // Nazwa urządzenia widoczna dla telefonu.
    ble_svc_gap_device_name_set("ESP32-Pedometer");

    // Rejestracja własnej usługi GATT (FFF0)
    // oraz charakterystyki licznika kroków (FFF1).
    ble_gatts_count_cfg(gatt_services);
    ble_gatts_add_svcs(gatt_services);

    printf("[BLE_Manager] GATT services registered.\n");

    // Uruchomienie NimBLE w osobnym zadaniu FreeRTOS.
    nimble_port_freertos_init(ble_host_task);

    printf("[BLE_Manager] NimBLE uruchomione poprawnie.\n");
}

// Wysyła aktualną liczbę kroków do telefonu przez BLE Notify.
void BLE_Manager::notifyStepCount()
{
    // Nie wysyłamy danych, jeśli telefon nie jest połączony.
    if (!device_connected)
    {
        return;
    }

    // Zamiana liczby kroków na tekst.
    char value[16];

    snprintf(value, sizeof(value), "%lu", current_step_count);

    // Utworzenie bufora BLE zawierającego dane do wysłania.
    struct os_mbuf* om = ble_hs_mbuf_from_flat(
        value,
        strlen(value)
    );

    if (om == nullptr)
    {
        return;
    }

    // Wysłanie Notify do podłączonego telefonu.
    ble_gatts_notify_custom(
        current_conn_handle,
        step_count_handle,
        om
    );
}

// Obsługa zdarzeń GAP (łączenie i rozłączanie urządzeń).
static int ble_gap_event(struct ble_gap_event* event, void* arg)
{
    switch (event->type)
    {
        case BLE_GAP_EVENT_CONNECT:

            // Próba połączenia zakończyła się sukcesem.
            if (event->connect.status == 0)
            {
                current_conn_handle = event->connect.conn_handle;
                device_connected = true;

                printf("[BLE_Manager] Device connected.\n");
            }
            else
            {
                printf("[BLE_Manager] Connection failed.\n");

                // Wracamy do advertising.
                start_advertising();
            }

            break;

        case BLE_GAP_EVENT_DISCONNECT:

            device_connected = false;

            printf("[BLE_Manager] Device disconnected.\n");

            // Po rozłączeniu ponownie reklamujemy urządzenie.
            start_advertising();

            break;

        default:
            break;
    }

    return 0;
}

// Uruchamia advertising BLE.
static void start_advertising(void)
{
    struct ble_hs_adv_fields fields;

    memset(&fields, 0, sizeof(fields));

    const char* device_name = "ESP32-Pedometer";

    // Nazwa widoczna podczas skanowania BLE.
    fields.name = (uint8_t*)device_name;
    fields.name_len = strlen(device_name);
    fields.name_is_complete = 1;

    ble_gap_adv_set_fields(&fields);

    struct ble_gap_adv_params adv_params;

    memset(&adv_params, 0, sizeof(adv_params));

    // Urządzenie może przyjmować połączenia.
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;

    // Urządzenie jest publicznie wykrywalne.
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

    // Rozpoczęcie advertising.
    ble_gap_adv_start(
        BLE_OWN_ADDR_PUBLIC,
        NULL,
        BLE_HS_FOREVER,
        &adv_params,
        ble_gap_event,
        NULL
    );

    printf("[BLE_Manager] Advertising started.\n");
}