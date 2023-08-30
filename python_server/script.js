document.addEventListener('DOMContentLoaded', () => {
    const fileList = document.getElementById('file-list');
    const uploadForm = document.getElementById('upload-form');

    // Function to fetch and update the file list
    function updateFileList() {
        fetch('/list')
            .then(response => response.text())
            .then(data => {
                fileList.innerHTML = data
                    .split('\n')
                    .map(fileName => {
                        if (fileName) {
                            return `
                                <li>${fileName}
                                    <button onclick="deleteFile('${fileName}')">Delete</button>
                                    <button onclick="renameFile('${fileName}')">Rename</button>
                                    <a href="/download/${fileName}" download>Download</a>
                                </li>`;
                        }
                    })
                    .join('');
            });
    }

    // Function to handle file deletion
    function deleteFile(fileName) {
        if (confirm(`Are you sure you want to delete ${fileName}?`)) {
            fetch(`/${fileName}`, { method: 'DELETE' })
                .then(response => response.text())
                .then(message => {
                    alert(message);
                    updateFileList();
                });
        }
    }

    // Function to handle file renaming
    function renameFile(fileName) {
        const newFileName = prompt(`Enter new name for ${fileName}:`);
        if (newFileName) {
            fetch(`/${fileName}?new_name=${newFileName}`, { method: 'PUT' })
                .then(response => response.text())
                .then(message => {
                    alert(message);
                    updateFileList();
                });
        }
    }

    // Update file list on page load
    updateFileList();

    // Handle file upload
    uploadForm.addEventListener('submit', async (event) => {
        event.preventDefault();
        const formData = new FormData(uploadForm);
        await fetch('/', { method: 'POST', body: formData });
        updateFileList();
        uploadForm.reset();
    });
});
