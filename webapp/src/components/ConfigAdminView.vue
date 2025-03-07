<template>
    <div class="container-xxl" role="main">
        <div class="page-header">
            <h1>Config Management</h1>
        </div>
        <BootstrapAlert v-model="showAlert" dismissible :variant="alertType">
            {{ alertMessage }}
        </BootstrapAlert>

        <div class="text-center" v-if="loading">
            <div class="spinner-border" role="status">
                <span class="visually-hidden">Loading...</span>
            </div>
        </div>

        <template v-if="!loading">
            <div class="card">
                <div class="card-header text-white bg-primary">Backup: Configuration File Backup</div>
                <div class="card-body text-center">
                    Backup the configuration file
                    <button class="btn btn-primary" @click="downloadConfig">Backup
                    </button>
                </div>
            </div>

            <div class="card mt-5">
                <div class="card-header text-white bg-primary">Restore: Restore the Configuration File</div>
                <div class="card-body text-center">

                    <div v-if="!uploading && UploadError != ''">
                        <p class="h1 mb-2">
                            <BIconExclamationCircleFill />
                        </p>
                        <span style="vertical-align: middle" class="ml-2">
                            {{ UploadError }}
                        </span>
                        <br />
                        <br />
                        <button class="btn btn-light" @click="clear">
                            <BIconArrowLeft /> Back
                        </button>
                    </div>

                    <div v-else-if="!uploading && UploadSuccess">
                        <span class="h1 mb-2">
                            <BIconCheckCircle />
                        </span>
                        <span> Upload Success </span>
                        <br />
                        <br />
                        <button class="btn btn-primary" @click="clear">
                            <BIconArrowLeft /> Back
                        </button>
                    </div>

                    <div v-else-if="!uploading">
                        <div class="form-group pt-2 mt-3">
                            <input class="form-control" type="file" ref="file" accept=".json" @change="uploadConfig" />
                        </div>
                    </div>

                    <div v-else-if="uploading">
                        <div class="progress">
                            <div class="progress-bar" role="progressbar" :style="{ width: progress + '%' }"
                                v-bind:aria-valuenow="progress" aria-valuemin="0" aria-valuemax="100">
                                {{ progress }}%
                            </div>
                        </div>
                    </div>

                    <div class="alert alert-danger mt-3" role="alert">
                        <b>Note:</b> This operation replaces the configuration file with the restored configuration and
                        restarts OpenDTU to apply all settings.
                    </div>
                </div>
            </div>

            <div class="card mt-5">
                <div class="card-header text-white bg-primary">Initialize: Perform Factory Reset</div>
                <div class="card-body text-center">

                    <button class="btn btn-danger" @click="onFactoryResetModal">Restore Factory-Default Settings
                    </button>

                    <div class="alert alert-danger mt-3" role="alert">
                        <b>Note:</b> Click Restore Factory-Default Settings to restore and initialize the
                        factory-default settings and reboot.
                    </div>
                </div>
            </div>
        </template>

        <div class="modal" id="factoryReset" tabindex="-1">
            <div class="modal-dialog">
                <div class="modal-content">
                    <div class="modal-header">
                        <h5 class="modal-title">Factory Reset</h5>
                        <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
                    </div>
                    <div class="modal-body">
                        Are you sure you want to delete the current configuration and reset all settings to their
                        factory defaults?
                    </div>
                    <div class="modal-footer">
                        <button type="button" class="btn btn-secondary" @click="onFactoryResetCancel"
                            data-bs-dismiss="modal">Cancel</button>
                        <button type="button" class="btn btn-danger" @click="onFactoryResetPerform">Factory
                            Reset!</button>
                    </div>
                </div>
            </div>
        </div>

    </div>
</template>

<script lang="ts">
import { defineComponent } from 'vue';
import * as bootstrap from 'bootstrap';
import BootstrapAlert from "@/components/partials/BootstrapAlert.vue";

export default defineComponent({
    components: {
        BootstrapAlert,
    },
    data() {
        return {
            modalFactoryReset: {} as bootstrap.Modal,
            alertMessage: "",
            alertType: "info",
            showAlert: false,
            loading: true,
            uploading: false,
            progress: 0,
            UploadError: "",
            UploadSuccess: false,
            file: {} as Blob,
        };
    },
    mounted() {
        this.modalFactoryReset = new bootstrap.Modal('#factoryReset');
        this.loading = false;
    },
    methods: {
        onFactoryResetModal() {
            this.modalFactoryReset.show();
        },
        onFactoryResetCancel() {
            this.modalFactoryReset.hide();
        },
        onFactoryResetPerform() {
            const formData = new FormData();
            formData.append("data", JSON.stringify({ delete: true }));

            fetch("/api/config/delete", {
                method: "POST",
                body: formData,
            })
                .then(function (response) {
                    if (response.status != 200) {
                        throw response.status;
                    } else {
                        return response.json();
                    }
                })
                .then(
                    (response) => {
                        this.alertMessage = response.message;
                        this.alertType = response.type;
                        this.showAlert = true;
                    }
                )
            this.modalFactoryReset.hide();
        },
        downloadConfig() {
            const link = document.createElement('a')
            link.href = "/api/config/get"
            link.download = 'config.json'
            link.click()
        },
        uploadConfig(event: Event | null) {
            this.uploading = true;
            const formData = new FormData();
            if (event !== null) {
                const target = event.target as HTMLInputElement;
                if (target.files !== null) {
                    this.file = target.files[0];
                }
            }
            const request = new XMLHttpRequest();
            request.addEventListener("load", () => {
                // request.response will hold the response from the server
                if (request.status === 200) {
                    this.UploadSuccess = true;
                } else if (request.status !== 500) {
                    this.UploadError = `[HTTP ERROR] ${request.statusText}`;
                } else {
                    this.UploadError = request.responseText;
                }
                this.uploading = false;
                this.progress = 0;
            });
            // Upload progress
            request.upload.addEventListener("progress", (e) => {
                this.progress = Math.trunc((e.loaded / e.total) * 100);
            });
            request.withCredentials = true;

            formData.append("config", this.file, "config");
            request.open("post", "/api/config/upload");
            request.send(formData);
        },
        clear() {
            this.UploadError = "";
            this.UploadSuccess = false;
        },
    },
});
</script>