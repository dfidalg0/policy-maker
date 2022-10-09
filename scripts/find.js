/**
 * Função útil para usar no console do browser em
 * https://chromium.googlesource.com/chromiumos/docs/+/master/constants/syscalls.md
 */
function find(/**@type {string}*/ name) {
    const table = document.querySelector(`h3:has(a[name="${name}"]) ~ table`);

    const rows = Array.from(table.querySelectorAll('tbody tr'));

    return rows.map(row => {
        const data = Array.from(row.children);

        const [
            nr,
            name,
            ,,
            ...args
        ] = data.map(cell => cell.textContent.trim());

        return {
            nr: Number(nr),
            name,
            args
        }
    });
}
